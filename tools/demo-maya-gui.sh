#!/bin/bash

# Maya OS Ubuntu Desktop Demo Script

echo "🚀 Starting Maya OS Ubuntu Desktop Demo..."

# Build if not already built
if [ ! -f maya-os.iso ]; then
	echo "Building Maya OS first..."
	./tools/build-ubuntu.sh
fi

echo ""
echo "🖱️  Launching Maya OS in QEMU..."
echo ""
echo "📚 Demo Instructions:"
echo "=================="
echo ""
echo "1. Boot Process:"
echo "   • Watch Maya OS boot with Ubuntu splash"
echo "   • Desktop loads with purple-orange gradient"
echo ""
echo "2. Desktop Features:"
echo "   • Unity-style launcher on the left"
echo "   • Top panel with Activities button"
echo "   • Desktop icons (Home, Documents, etc.)"
echo "   • System tray with time and indicators"
echo ""
echo "3. Navigation:"
echo "   • Use ARROW KEYS to move mouse cursor"
echo "   • Press SPACE to click/select items"
echo "   • Press SPACE on 'Activities' to open app dash"
echo ""
echo "4. Applications to Try:"
echo "   • Files - Browse filesystem"
echo "   • Terminal - Command line interface"
echo "   • Text Editor - Write and edit text"
echo "   • Calculator - Perform calculations"
echo "   • Settings - Configure system"
echo ""
echo "5. Ubuntu Features:"
echo "   • Search applications in dash"
echo "   • Notifications appear in top-right"
echo "   • Window management with title bars"
echo "   • System menu with shutdown/restart"
echo ""
echo "Press any key to start the demo..."
read -n 1 -s

# Launch QEMU with optimal settings for demo
qemu-system-i386 \
	-cdrom maya-os.iso \
	-m 256M \
	-enable-kvm \
	-display gtk \
	-name "Maya OS - Ubuntu Desktop" \
	-boot d

echo ""
echo "Demo completed! 🎉"
echo ""
echo "To run Maya OS again:"
echo "  make run"
echo ""
echo "To modify the Ubuntu desktop:"
echo "  • Edit files in gui/ directory"
echo "  • Customize colors in include/gui/ubuntu_desktop.h"
echo "  • Add new applications in gui/ubuntu_apps.c"
echo ""