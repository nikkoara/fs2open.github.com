#!/bin/bash

glad --api "gl=3.2" --generator c --profile core --no-loader --extensions extensions.txt --spec gl --out-path glad
glad --spec wgl --generator c --extensions wgl_exts.txt --out-path glad_wgl --no-loader
