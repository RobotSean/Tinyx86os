
project(boot LANGUAGES C)

# 使用自定义的链接器
#-m elf_i386：指示链接器使用 32 位 ELF 格式，即生成的目标文件是 32 位的 ELF 格式文件。
#-Ttext=0x7c00：指定将 代码段（.text）加载到内存的 0x7c00 地址上。这个地址是传统的引导扇区地址，也就是引导程序启动时加载到内存的地方。
#--section-start boot_end=0x7dfe：设置一个 自定义段（boot_end）的起始地址为 0x7dfe，即该段会被放置在内存的 0x7dfe 地址。这通常用于指定 引导程序的结束位置。
set(CMAKE_EXE_LINKER_FLAGS "-m elf_i386  -Ttext=0x7c00  --section-start boot_end=0x7dfe")
#<OBJECTS>：表示所有的目标文件（.o 文件），会由 CMake 自动处理。
set(CMAKE_C_LINK_EXECUTABLE "${LINKER_TOOL} <OBJECTS> ${CMAKE_EXE_LINKER_FLAGS} -o ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.elf")

# 将所有的汇编、C文件加入工程
# 注意保证start.asm在最前头，这样对应的程序会放在bin文件开头，从而在加载到0x7c00时能在开头处
file(GLOB C_LIST "*.c" "*.h")
add_executable(${PROJECT_NAME} start.S ${C_LIST})

# bin文件生成，写入到image目录下
#POST_BUILD：表示这些命令会在目标 boot 可执行文件（PROJECT_NAME）构建完成后执行。
add_custom_command(TARGET ${PROJECT_NAME}
                   POST_BUILD
                   COMMAND ${OBJCOPY_TOOL} -O binary ${PROJECT_NAME}.elf ${CMAKE_SOURCE_DIR}/image/${PROJECT_NAME}.bin
                   #生成反汇编
                   COMMAND ${OBJDUMP_TOOL} -x -d -S -m i8086 ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.elf > ${PROJECT_NAME}_dis.txt
                   #使用 readelf 工具来显示 ELF 文件的详细信息。包含 ELF 文件头、节头、段信息等详细数据
                   COMMAND ${READELF_TOOL} -a ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.elf > ${PROJECT_NAME}_elf.txt
)