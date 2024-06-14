for file in *.cpp; do
  filename=$(basename "$file" .cpp)
  
  g++ -DPSYLEAGUE -std=c++17 "$file" -o $filename
done