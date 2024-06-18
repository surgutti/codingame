python3 coef_gen.py
for file in gen_bots/*.cpp; do
    filename=$(basename "$file" .cpp)

    psyleague bot add $filename -s gen_bots/$filename
done