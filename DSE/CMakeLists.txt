add_llvm_library( LLVMSimpleDSE MODULE BUILDTREE_ONLY
  DSE.cpp

  DEPENDS
  intrinsics_gen
  PLUGIN_TOOL
  opt
  )
