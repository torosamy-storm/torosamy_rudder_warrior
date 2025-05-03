bin_directory=$(pwd)

cd $bin_directory

rm -rf out
mkdir out

cp -rf torosamy_automatic_aiming out
cp -rf torosamy_robomaster_sdk out
cp -rf build.sh out
cp -rf package.sh out
cp -rf start.sh out
cp -rf version.txt out
rm -rf out/torosamy_automatic_aiming/build
rm -rf out/torosamy_robomaster_sdk/resource/video/*.mp4

cd out


tar -czvf "../torosamy_rudder_warrior.tar.gz" .

cd ..
mv "torosamy_rudder_warrior.tar.gz" out