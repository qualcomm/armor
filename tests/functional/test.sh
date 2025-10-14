
# Root directory containing all test folders
ROOT_DIR="."

for dir in $ROOT_DIR/*; do
    test_case_dir=$(basename "$dir")
    if [ ! -d "$test_case_dir" ]; then
        continue
    fi
    
    echo "Test Case: $test_case_dir"
    
    cd $test_case_dir

    ./../../../build/lib_ast_tree_compare -p=../../../build mylib.h mylib_v2.h
    
    cd ..

done
