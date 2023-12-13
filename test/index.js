"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
const cjs_1 = require("../dist/cjs");
const node_fs_1 = __importDefault(require("node:fs"));
const main = async () => {
    const data = node_fs_1.default.readFileSync('./images/test01.jpg');
    const encoded = await (0, cjs_1.optimizeImage)({ image: data, width: 300, height: 200, quality: 100 });
    if (encoded) {
        node_fs_1.default.writeFileSync('./test2.webp', encoded);
    }
};
main();
