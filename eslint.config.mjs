/**
 * @type {import('eslint').Linter.Config[]}
 */
import eslint from "@eslint/js";
import eslintConfigPrettier from "eslint-config-prettier";
import importPlugin from "eslint-plugin-import";
import tslint from "typescript-eslint";

export default [
  eslint.configs.recommended,
  ...tslint.configs.recommended,
  eslintConfigPrettier,
  {
    ignores: ["**/global-types.ts"],
  },
  {
    plugins: {
      import: importPlugin,
    },
    rules: {
      "@typescript-eslint/no-unused-vars": 0,
      "no-empty-pattern": 0,
      "no-empty": 0,
      "import/order": [
        "warn",
        {
          groups: [
            "builtin",
            "external",
            "internal",
            ["parent", "sibling"],
            "object",
            "type",
            "index",
          ],
          pathGroupsExcludedImportTypes: ["builtin"],
          alphabetize: {
            order: "asc",
            caseInsensitive: true,
          },
        },
      ],
    },
  },
];
