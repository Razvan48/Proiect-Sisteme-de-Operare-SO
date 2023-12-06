SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

gcc "$SCRIPT_DIR/da.cpp" -o "$SCRIPT_DIR/da"

echo $SCRIPT_DIR

if [ ":$PATH:" != *":$SCRIPT_DIR:"* ]; then
	echo "export PATH=\$PATH:$SCRIPT_DIR" >> ~/.bashrc
	export PATH=$PATH:$SCRIPT_DIR
fi

echo $PATH
