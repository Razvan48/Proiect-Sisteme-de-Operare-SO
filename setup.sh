SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

gcc "$SCRIPT_DIR/da.cpp" -o "$SCRIPT_DIR/da"

echo "da.cpp has been compiled."

echo $SCRIPT_DIR

if echo $PATH | grep -q "$SCRIPT_DIR"; then
	echo "Directory is already in the PATH."
else
	echo "export PATH=\$PATH:$SCRIPT_DIR" >> ~/.bashrc
	export PATH=$PATH:$SCRIPT_DIR
	echo "Directory added to PATH."
	echo "Please reboot."
fi
