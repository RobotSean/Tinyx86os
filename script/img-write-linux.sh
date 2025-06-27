if [ -f "disk1.vhd" ]; then
    mv disk1.vhd disk1.img
fi

if [ ! -f "disk1.img" ]; then
    echo "error: no disk1.vhd, download it first!!!"
    exit -1
fi

if [ -f "disk2.vhd" ]; then
    mv disk2.vhd disk2.img
fi

if [ ! -f "disk2.img" ]; then
    echo "error: no disk2.vhd, download it first!!!"
    exit -1
fi

export DISK1_NAME=disk1.img

# 写boot区，定位到磁盘开头，写1个块：512字节
dd if=boot.bin of=$DISK1_NAME bs=512 conv=notrunc count=1

#写loader区，定位到磁盘第2个块
dd if=loader.bin of=$DISK1_NAME bs=512 conv=notrunc seek=1

#写kernel区，定位到磁盘第100个块
dd if=kernel.elf of=$DISK1_NAME bs=512 conv=notrunc seek=100

# 写应用程序init，临时使用
# # dd if=init.elf of=$DISK1_NAME bs=512 conv=notrunc seek=5000
# dd if=shell.elf of=$DISK1_NAME bs=512 conv=notrunc seek=5000

#写应用程序，使用系统的挂载命令
export DISK2_NAME=disk2.img
export TARGET_PATH=mp
#设置挂载点目录为 mp
#强制删除挂载点目录（如果已经存在），避免残留数据或挂载失败。
sudo rm -rf $TARGET_PATH
mkdir $TARGET_PATH
#将 disk2.img 中偏移 128 个扇区（每个扇区 512 字节，即从第 65536 字节起）的文件系统挂载到 mp 目录
sudo mount -o offset=$[128*512],rw $DISK2_NAME $TARGET_PATH
# sudo cp -v init.elf $TARGET_PATH/init
# sudo cp -v shell.elf $TARGET_PATH
# sudo cp -v loop.elf $TARGET_PATH/loop
sudo cp -v *.elf $TARGET_PATH
sudo umount $TARGET_PATH
