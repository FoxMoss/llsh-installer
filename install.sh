OS=$(uname -s)
ARCH=$(uname -m)
if [ "$OS" != "Linux" ]; then
  echo "Linux is currently only supported"
  exit
fi

if [ "$ARCH" != "x86_64" ]; then
  echo "x86_64 is currently only supported"
  exit
fi

curl "https://github.com/FoxMoss/llsh-installer/releases/download/v0.0.1/$ARCH-$OS-installer" --output /tmp/llsh-installer -# -L
chmod +x /tmp/llsh-installer
/tmp/llsh-installer
rm /tmp/llsh-installer
