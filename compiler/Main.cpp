// This file is part of SymCC.
//
// SymCC is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// SymCC is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// SymCC. If not, see <https://www.gnu.org/licenses/>.

#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>

#include "Pass.h"



#if LLVM_VERSION_MAJOR >= 9 && LLVM_VERSION_MAJOR < 12

void addSymbolizePass(const llvm::PassManagerBuilder & /* unused */,
                      llvm::legacy::PassManagerBase &PM) {
  PM.add(new SymbolizePass());
}

// Make the pass known to opt.
static llvm::RegisterPass<SymbolizePass> X("symbolize", "Symbolization Pass");
// Tell frontends to run the pass automatically.
static struct llvm::RegisterStandardPasses
    Y(llvm::PassManagerBuilder::EP_VectorizerStart, addSymbolizePass);
static struct llvm::RegisterStandardPasses
    Z(llvm::PassManagerBuilder::EP_EnabledOnOptLevel0, addSymbolizePass);


#else
//-----------------------------------------------------------------------------
// New PM Registration
//-----------------------------------------------------------------------------
llvm::PassPluginLibraryInfo getSymccPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "Symcc", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            //errs() << "registerPipelineStartEPCallback" << "\n";
            PB = PB;
            PB.registerPipelineStartEPCallback(
              [](ModulePassManager &MPM, PassBuilder::OptimizationLevel Level) {
                 FunctionPassManager FPM;
                 Level = Level; //hack for now
                 MPM.addPass(SymbolizePass());
                 MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
            });
         }};
}

// This is the core interface for pass plugins. It guarantees that 'opt' will
// be able to recognize SEP when added to the pass pipeline on the
// command line, i.e. via '-passes=hello-world'
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getSymccPluginInfo();
}

#endif
