SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
echo $SCRIPT_DIR

g++ "$SCRIPT_DIR/da.cpp" -o "$SCRIPT_DIR/da" -std=c++11 -pthread -lrt
g++ "$SCRIPT_DIR/the_daemon.cpp" -o "$SCRIPT_DIR/the_daemon" -std=c++11 -pthread -lrt
echo "project compiled"

IFS=":" read -ra path_dirs <<< "$PATH"

directory_found=false

for dir in "${path_dirs[@]}"; do
    echo "$dir"
    if [[ "$dir" == "$SCRIPT_DIR" ]]; then
        directory_found=true
        break
    fi
done

if [ "$directory_found" = true ]; then
	echo "Directory is already in the PATH"
else
	echo "" >> ~/.bashrc
	echo "export PATH="$SCRIPT_DIR:$PATH"" >> ~/.bashrc
	echo "" >> ~/.bashrc

	export PATH="$SCRIPT_DIR:$PATH"

	echo "Directory added to the PATH"

	while true; do
	    read -p "Restart the terminal"
	done
fi

