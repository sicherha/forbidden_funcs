// forbidden_funcs - a GCC plugin checking for calls to forbidden functions
// Copyright (C) 2022  Christoph Erhardt
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include <string>
#include <unordered_set>

// clang-format off
#include <gcc-plugin.h>
#include <basic-block.h>
#include <tree.h>
#include <tree-ssa-alias.h>
#include <gimple-expr.h>
#include <gimple.h>
// clang-format on

#include <context.h>
#include <diagnostic-core.h>
#include <gimple-iterator.h>
#include <plugin-version.h>
#include <tree-pass.h>

namespace {

plugin_info pluginInfo = {
    .version = "1.0",
    .help =
        "Treats calls to forbidden functions as `-Wdeprecated`.\n"
        "Pass a comma-separated list of (mangled) symbols as argument, e.g.\n"
        "  `-fplugin-arg-forbidden_funcs-list=func1,func2,func3`.",
};

void extractCommaSeparatedItems(std::unordered_set<std::string>& outSet,
                                const char inString[]) {
  const char* start = inString;
  const char* end;
  for (end = start; *end != '\0'; ++end) {
    if (*end == ',') {
      if (end - start > 0) {
        outSet.insert(std::string(start, end));
      }
      start = end + 1;
    }
  }
  if (end - start > 0) {
    outSet.insert(std::string(start, end));
  }
}

std::unordered_set<std::string> parseArgs(const plugin_name_args* nameArgs) {
  std::unordered_set<std::string> forbiddenFuncs;

  for (int i = 0; i < nameArgs->argc; ++i) {
    const auto& arg = nameArgs->argv[i];

    if (strcmp(arg.key, "list") == 0 && arg.value) {
      extractCommaSeparatedItems(forbiddenFuncs, arg.value);
      continue;
    }

    error(
        "unrecognized command-line option %<-%s%s%> for plugin %s; "
        "did you mean %<-list=%>?",
        arg.key,
        arg.value ? "=" : "",
        nameArgs->base_name);
  }

  return forbiddenFuncs;
}

constexpr pass_data passData = {
    .type = GIMPLE_PASS,
    .name = "forbidden_funcs",
    .optinfo_flags = OPTGROUP_NONE,
    .tv_id = TV_NONE,
    .properties_required = PROP_gimple_any,
    .properties_provided = 0,
    .properties_destroyed = 0,
    .todo_flags_start = 0,
    .todo_flags_finish = 0,
};

class ForbiddenFunctionCheck final : public gimple_opt_pass {
 private:
  ForbiddenFunctionCheck(gcc::context* ctx,
                         std::unordered_set<std::string> forbiddenFuncs)
      : gimple_opt_pass(passData, ctx),
        m_forbiddenFuncs(std::move(forbiddenFuncs)) {}

  unsigned execute(function* function) override {
    basic_block bb;
    FOR_ALL_BB_FN(bb, function) {
      for (auto gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
        const auto* statement = gsi_stmt(gsi);
        if (gimple_code(statement) != GIMPLE_CALL) {
          continue;
        }
        const auto callee = gimple_call_fndecl(statement);
        if (!callee) {
          continue;
        }
        const auto* calleeSymbol =
            IDENTIFIER_POINTER(DECL_ASSEMBLER_NAME(callee));
        if (m_forbiddenFuncs.find(calleeSymbol) == m_forbiddenFuncs.end()) {
          continue;
        }

        warning_at(gimple_location(statement),
                   OPT_Wdeprecated,
                   "call to forbidden function %qD",
                   callee);
      }
    }
    return 0;
  }

 public:
  static void createAndRegister(
      gcc::context* ctx,
      const std::unordered_set<std::string>& forbiddenFuncs,
      const plugin_name_args* nameArgs) {
    // Create the pass
    auto* pass = new ForbiddenFunctionCheck(ctx, forbiddenFuncs);

    // Register it
    register_pass_info passInfo = {
        .pass = pass,
        .reference_pass_name = "ssa",
        .ref_pass_instance_number = 1,
        .pos_op = PASS_POS_INSERT_AFTER,
    };
    register_callback(
        nameArgs->base_name, PLUGIN_PASS_MANAGER_SETUP, nullptr, &passInfo);

    // Ensure proper destruction on shutdown
    register_callback(
        nameArgs->base_name,
        PLUGIN_FINISH,
        [](void*, void* userData) {
          delete static_cast<ForbiddenFunctionCheck*>(userData);
        },
        pass);
  }

 private:
  std::unordered_set<std::string> m_forbiddenFuncs;
};

}  // namespace

int plugin_is_GPL_compatible;

int plugin_init(plugin_name_args* nameArgs, plugin_gcc_version* version) {
  // Check version
  if (!plugin_default_version_check(version, &gcc_version)) {
    fprintf(stderr,
            "Plugin %s was built for GCC version %d.%d\n",
            nameArgs->base_name,
            GCCPLUGIN_VERSION_MAJOR,
            GCCPLUGIN_VERSION_MINOR);
    return 1;
  }

  // Set plugin info
  register_callback(nameArgs->base_name, PLUGIN_INFO, nullptr, &pluginInfo);

  // Create and register analysis pass
  const auto forbiddenFuncs = parseArgs(nameArgs);
  ForbiddenFunctionCheck::createAndRegister(g, forbiddenFuncs, nameArgs);

  return 0;
}
