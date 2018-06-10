1. Open Vivado
2. Use the tcl console to navigate to the directory containing the project.tcl script
	ie: "cd C:/Users/wesleyk/Desktop/Ensc452/project_demo/scripts"
3. Use the tcl console to source the script: "source ./project.tcl"

The project with the relevant block diagram will be created.
Tested only on Windows using Vivado 2017.3.

Build the project (clock "Generate Bitstream"). Then export the hardware with bitstream included (local to project) and launch the SDK (also local to project).
Make a "hello world" application. You will then need to copy the content of the "software" directory to the project; drag and drop each file and folder in the ../software directory to the project.
Delete "hello_world.c".

