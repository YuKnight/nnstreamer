tensor_merge_sources = [
    'gsttensormerge.c'
]

tensor_mergeOBJ = static_library('tensor_merge',
    tensor_merge_sources,
    c_args: nnstreamer_base_args,
    dependencies: [
        dependencies
    ],
    include_directories: inc
).extract_all_objects()

objects += tensor_mergeOBJ
