#/bin/bash
fs_mnt_dir=/tmp/fs_mnt
for i in $(find $fs_mnt_dir/apps/ -iname '*.o');
do
	echo $i;
	mv $i `echo $i | cut -d '.' -f1`;
done



