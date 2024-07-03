add_llvm_library( LLVMLICM MODULE BUILDTREE_ONLY
  LICM.cpp
  InstructionsSet.cpp

  DEPENDS
  intrinsics_gen
  PLUGIN_TOOL
  opt
  )
