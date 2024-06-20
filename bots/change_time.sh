find "." -type f -name "*.cpp" | while read -r file; do
    # Use sed to replace the line
    sed -i 's/brain.run(current_state, 45);/brain.run(current_state, 20);/' "$file"
    sed -i 's/mcts.run(current_state, 45);/mcts.run(current_state, 20);/' "$file"
done
