N=`ls *.files`
echo Makefile > $N
echo TODO.md >> $N
echo update_files.sh >> $N
echo make_class.met >> $N
# echo TODO.txt >> $N
for d in bin met src tests
do
    for t in '*.h' '*.cpp' '*.cu' '*.txt' '*.py' '*.js' '*.html' '*.css' '*.files' '*.met' '*.coffee' '*.asm'
    do
        for i in `find $d -name "$t" -a -not -wholename "*/compilations/*"`
        do
            echo $i >> $N
            echo $i
            # git add $i
        done
    done
done
