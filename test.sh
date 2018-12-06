declare -a arr=("720" "1080" "1440" "1920" "2560")

## now loop through the above array
for i in "${arr[@]}"
do
   INPUT_FILE="benchmark/$i.bmp"
   OUTPUT_FILE="output/$i.jpg"
   echo "image: $INPUT_FILE"
   ./jpeg_encoder $INPUT_FILE $OUTPUT_FILE $1
   # or do whatever with individual element of the array
done
