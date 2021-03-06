/*
 * Copyright (c) 2018 Jie Zheng
 */
#include <kernel/include/printk.h>
#include <device/include/ata.h>
#include <x86/include/ioport.h>
#include <memory/include/malloc.h>
#include <lib/include/string.h>

static struct list_elem ata_device_list;

char * device_type_str[] = {
    "UNDEFINED_DEVICE_TYPE",
    "PATA_DEVICE",
    "SATA_DEVICE",
    "PATAPI_DEVICE",
    "SATAPI_DEVICE"

};
void
dump_ata_devices(void)
{
    struct list_elem * _list;
    struct ata_device * _device;
    LOG_INFO("Dump ATA family devices:\n");
    LIST_FOREACH_START(&ata_device_list, _list) {
        _device = CONTAINER_OF(_list, struct ata_device, list);
        LOG_INFO("   %s on %s bus as %s drive: io_base:0x%x, ctrl_base:0x%x\n",
            device_type_str[_device->type],
            _device->bus == ATA_PRIMARY ? "primary" : "secondary",
            _device->drive == ATA_MASTER ? "master" : "slave",
            _device->io_base,
            _device->ctrl_base);
    }
    LIST_FOREACH_END();
}
void
ata_delay(uint8_t bus)
{
    uint16_t port = bus == ATA_MASTER?
        ATA_PRIMARY_BUS_IO_BASE:
        ATA_SECONDARY_BUS_IO_BASE;
    int idx = 0;
    port += STATUS_REGISTER_OFFSET;
    for(idx = 0; idx < 4; idx++)
        inb(port);
}


int32_t
select_drive(uint8_t bus, uint8_t drive)
{
    uint8_t drive_byte = drive == ATA_MASTER ? 0xa0 : 0xb0;
    uint16_t port = bus == ATA_PRIMARY ?
        ATA_PRIMARY_BUS_IO_BASE:
        ATA_SECONDARY_BUS_IO_BASE;
    port += DRIVER_REGISTER_OFFSET;
    ASSERT(bus == ATA_PRIMARY || bus == ATA_SECONDARY);
    ASSERT(drive == ATA_MASTER || drive == ATA_SLAVE);
    outb(port, drive_byte);
    return OK;
}

void
identify_drive(uint8_t bus, uint8_t drive)
{
    uint16_t io_base = bus == ATA_PRIMARY ?
        ATA_PRIMARY_BUS_IO_BASE:
        ATA_SECONDARY_BUS_IO_BASE;
    uint16_t ctrl_base = bus == ATA_PRIMARY ?
        ATA_PRIMARY_BUS_CONTROL_BASE:
        ATA_SECONDARY_BUS_CONTROL_BASE;
    uint8_t status = 0;
    uint8_t mid = 0;
    uint8_t high = 0;
    uint8_t device_type = UNDEFINED_DEVICE_TYPE;
    struct ata_device * _device;
    /*
     * Reset the bus and wait until master drive is ready
     */
    outb(io_base + DRIVER_REGISTER_OFFSET, (drive == ATA_MASTER) ? 0xa0 : 0xb0);
    outb(ctrl_base, 0);

    outb(io_base + DRIVER_REGISTER_OFFSET, 0xa0);
    status = inb(io_base + STATUS_REGISTER_OFFSET);
    if(status == 0xff) {
        LOG_WARN("Identifying %s bus %s drive fails: Floating Bus\n",
            bus == ATA_PRIMARY? "primary" : "secondary",
            drive == ATA_MASTER? "master" : "slave");
        return;
    }
    /*
     *Send IDENTIFY command to the drive
     */
    outb(io_base + DRIVER_REGISTER_OFFSET, (drive == ATA_MASTER) ? 0xa0 : 0xb0);
    outb(io_base + SECTOR_COUNTER_REGISTER_OFFSET, 0);
    outb(io_base + LBA_LOW_OFFSET, 0);
    outb(io_base + LBA_MID_OFFSET, 0);
    outb(io_base + LBA_HIGH_OFFSET, 0);
    outb(io_base + COMMAND_REGISTER_OFFSET, CMD_IDENTITY);
    status = inb(io_base + STATUS_REGISTER_OFFSET);
    if(status == 0x00) {
        LOG_WARN("Identifying %s bus %s drive fails: Drive Not Present\n",
            bus == ATA_PRIMARY? "primary" : "secondary",
            drive == ATA_MASTER? "master" : "slave");
        return;
    }
    while((status = inb(io_base + STATUS_REGISTER_OFFSET)) & STATUS_BUSY);
    mid = inb(io_base + LBA_MID_OFFSET);
    high = inb(io_base + LBA_HIGH_OFFSET);
    if (mid == 0x14 && high == 0xeb)
        device_type = PATAPI_DEVICE;
    else if (mid == 0x69 && high == 0x96)
        device_type = SATAPI_DEVICE;
    else if (mid == 0x3c && high == 0xc3)
        device_type =SATA_DEVICE;
    else if (mid == 0x0 && high == 0x0)
        device_type = PATA_DEVICE;
    LOG_DEBUG("Indentifying %s bus %s drive as: %s\n",
        bus == ATA_PRIMARY? "primary" : "secondary",
        drive == ATA_MASTER? "master" : "slave",
        device_type_str[device_type]);
    /*
     * Make a device entry
     */
    _device = malloc(sizeof(struct ata_device));
    memset(_device, 0x0, sizeof(struct ata_device));
    _device->io_base = io_base;
    _device->ctrl_base = ctrl_base;
    _device->bus = bus;
    _device->drive = drive;
    _device->type = device_type;
    list_append(&ata_device_list, &_device->list);
}

void
reset_ata_device(struct ata_device * _device)
{
    outb(_device->ctrl_base, CMD_RESET);
    ata_delay(_device->bus);
    outb(_device->ctrl_base, 0x0);
    ata_delay(_device->bus);
}

void
disable_interrupt(struct ata_device * _device)
{
    outb(_device->ctrl_base, CMD_DISABLE_INTERRUPT);
    ata_delay(_device->bus);
}

/*
 * CAVEAT: This will clean any state of the drive
 * make sure the SRST and HOB will not be influenced.
 */
void
enable_interrupt(struct ata_device * _device)
{
    outb(_device->ctrl_base, 0);
    ata_delay(_device->bus);
}
/*
 * Poll devcie status util BUSY bit is clear and DRQ is set, return OK
 * if ERR is set or DF(DRIVE FAULT) is set, return a negative interger
 */
inline int
ata_device_poll(struct ata_device * _device)
{
    uint8_t status  = 0;
    do {
        status = inb(_device->io_base + STATUS_REGISTER_OFFSET);
        if (status & STATUS_ERROR || status & STATUS_DRIVE_FAULT)
            return -ERR_DEVICE_FAULT;
        if (!(status & STATUS_BUSY) && status & STATUS_DATA_REQUEST)
            break;
    } while(1);
    return OK;
}
/*
 * here we use PIO LBA 28bit mode to write to sectores
 * note the count is supposed to lower than 256*512 = 128KB
 * in which case, we need a wrapper to split write operation into
 * mutiple one
 */
int
ata_device_write_one_sector(struct ata_device * _device,
    uint32_t LBA28,
    void * buffer)
{
    uint16_t * ptr = (uint16_t * )buffer;
    int idx = 0;
    uint8_t status;
    outb(_device->io_base + DRIVER_REGISTER_OFFSET,
        ((_device->drive == ATA_MASTER) ? 0xe0 : 0xf0) |
        ((LBA28 >> 24) & 0x0f));

    outb(_device->io_base + ERROR_REGISTER_OFFSET, 0x0);
    outb(_device->io_base + SECTOR_COUNTER_REGISTER_OFFSET, 1);
    outb(_device->io_base + LBA_LOW_OFFSET, (uint8_t)LBA28);
    outb(_device->io_base + LBA_MID_OFFSET, (uint8_t)(LBA28 >> 8));
    outb(_device->io_base + LBA_HIGH_OFFSET, (uint8_t)(LBA28 >> 16));
    outb(_device->io_base + COMMAND_REGISTER_OFFSET, CMD_WRITE_SECTORS);
    while((status = inb(_device->io_base + STATUS_REGISTER_OFFSET)) & STATUS_BUSY);
    //ata_device_poll(_device);
    for(idx = 0; idx < 256; idx++) {
        outw(_device->io_base + DATA_REGISTER_OFFSET, ptr[idx]);
    }
    return OK;
}

void
ata_device_flush_cache(struct ata_device * _device)
{
    outb(_device->io_base + DRIVER_REGISTER_OFFSET,
        _device->drive == ATA_MASTER ? 0xe0 : 0xf0);
    outb(_device->io_base + COMMAND_REGISTER_OFFSET, CMD_FLUSH_CACHE);

    ata_device_poll(_device);
}
/*
 * the read counterpart of ata_device_write_sectors.
 */
int
ata_device_read_one_sector(struct ata_device * _device,
    uint32_t LBA28,
    void * buffer)
{
    int idx = 0;
    uint16_t * ptr = (uint16_t *)buffer;
    uint8_t status;
    outb(_device->io_base + DRIVER_REGISTER_OFFSET,
        ((_device->drive == ATA_MASTER) ? 0xe0 : 0xf0) |
        ((LBA28 >> 24) & 0x0f));
    outb(_device->io_base + ERROR_REGISTER_OFFSET, 0x0);
    outb(_device->io_base + SECTOR_COUNTER_REGISTER_OFFSET, 1);
    outb(_device->io_base + LBA_LOW_OFFSET, (uint8_t)LBA28);
    outb(_device->io_base + LBA_MID_OFFSET, (uint8_t)(LBA28 >> 8));
    outb(_device->io_base + LBA_HIGH_OFFSET, (uint8_t)(LBA28 >> 16));
    outb(_device->io_base + COMMAND_REGISTER_OFFSET, CMD_READ_SECTORS);
    //ata_device_poll(_device);
    while((status = inb(_device->io_base + STATUS_REGISTER_OFFSET)) & STATUS_BUSY);
    for(idx = 0; idx < 256; idx++) {
        ptr[idx] = inw(_device->io_base + DATA_REGISTER_OFFSET);
    }
    return OK;
}

void
ata_init(void)
{
    list_init(&ata_device_list);
    identify_drive(ATA_PRIMARY, ATA_MASTER);
    identify_drive(ATA_PRIMARY, ATA_SLAVE);
    identify_drive(ATA_SECONDARY, ATA_MASTER);
    identify_drive(ATA_SECONDARY, ATA_SLAVE);
    dump_ata_devices();
}
