all:
	for i in `find . -name "*.coffee"`; do coffee -b -c $$i; done
	xdotool search "asm.js" windowactivate key F5 || firefox test.html
# 	nodejs test.js
	
	