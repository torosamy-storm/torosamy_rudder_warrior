bin_directory=$(pwd)

cd $bin_directory
current_time=$(date +"%Y-%m-%d_%H-%M-%S")
echo "${current_time}" > version.txt
echo "${current_time}"

cd torosamy_automatic_aiming





if [ ! -d "build" ]; then
    mkdir build
fi
cd build
cmake ..
make
