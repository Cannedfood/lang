rm -R -- projects/*/

premake5 gmake2 --location=projects/gmake &
premake5 vs2019 --location=projects/vs2019 &

wait
