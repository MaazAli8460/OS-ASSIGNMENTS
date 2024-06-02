#!/bin/bash

choice=-1

while true;
do
	echo "Menu:"
	echo "1. Get Processor Info"
	echo "2. Create a Testing Directory"
	echo "3. Safe Delete"
	echo "4. MinEdit: A Minimalist CLI-Based C Program Editor "
	echo "5. Exit"
	read -p "Enter your choice: " choice
	echo "You selected Option: $choice"

	case $choice in
        1)
            chmod +x cpu_info.sh
            ./cpu_info.sh
            #echo "docker run -v "$(pwd):/app" your_docker_username/assignment_01_section_X_sp2024_os /bin/bash -c "./cpu_info.sh; cat CPU_info.txt""
            ;;
        2)
            chmod +x GenData.sh
            ./GenData.sh
            #docker run -v "$(pwd):/app" your_docker_username/assignment_01_section_X_sp2024_os /bin/bash -c "./create_test_directory.sh"
            ;;
        3)
            read -p "Enter filename to safe delete: " filename
            chmod +x safeDel.sh
            ./safeDel.sh $filename
            #read -p "Enter filename to delete: " filename
            #docker run -v "$(pwd):/app" your_docker_username/assignment_01_section_X_sp2024_os /bin/bash -c "./SafeRemove.sh $filename"
            ;;
        4)
            #docker run -v "$(pwd):/app" your_docker_username/assignment_01_section_X_sp2024_os /bin/bash -c "./MinEdit.sh"
            g++ MinEdit.c -o MinEditE
            ./MinEditE

            ;;
        5)
            #echo "Exiting..."
            break
            ;;
        *)
            echo "Invalid choice. Please try again."
            ;;
    esac
done
