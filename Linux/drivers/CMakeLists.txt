# CMakeList.txt: drivers 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
# 由于我的系统更新过内核，所以这里需要设置内核源代码路径
set(KERNEL_DIR /home/nagisa/linux/course)
set(OUTPUT_DIR /home/nagisa/linux/nfs/drivers)

# 将源代码添加到此项目的可执行文件。
file(GLOB DRIVER_SOURCE "./*.cpp" "./*.c")

add_executable (test ${DRIVER_SOURCE})

# 定义编译函数/方法，驱动编译过程主要在这个函数进行
function(compile_module obj)
	set(TARGET_NAME ${obj})
	add_custom_target(${TARGET_NAME} ALL cp -f ${CMAKE_CURRENT_SOURCE_DIR}/*.c ${CMAKE_CURRENT_BINARY_DIR}/
  		COMMAND echo "compiling module ${obj}.ko..."
  	)
	
	list(LENGTH ARGN argn_len)
	set(i 0)
	set(depend_objlist "")
	while( i LESS ${argn_len})  
		list(GET ARGN ${i} argn_value)  
		set(depend_objlist "${depend_objlist} ${argn_value}.o") 
		math(EXPR i "${i} + 1")  
	endwhile() 
  
	add_custom_command(TARGET ${TARGET_NAME}
		POST_BUILD
		COMMAND echo "obj-m := ${obj}.o" > ${CMAKE_CURRENT_BINARY_DIR}/Makefile
		COMMAND echo "${obj}-objs:=${depend_objlist}" >>${CMAKE_CURRENT_BINARY_DIR}/Makefile
		COMMAND make -C ${KERNEL_DIR} M=${CMAKE_CURRENT_BINARY_DIR} modules
	)
endfunction()

add_subdirectory ("esp32")
