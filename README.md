ugles2
======

ugles2 は、OpenGL ES 2 を使用するための小さなライブラリです。

依存ライブラリ
--------------

* libpng (option)
* libjpeg (option)
* freetype (option)

※ configure 時に使用するライブラリを選択可


ビルド
------

sample 以下の Makefile に例があります。

### 例. 1 Debian (MESA)

    # aptitude install libgles2-mesa-dev libpng-dev libjpeg-dev libfreetype6-dev
    $ ./configur --prefix=/path/to/install --enable-png --enable-jpeg --enable-freetype --with-includes=/usr/include/freetype2
    $ make
    $ make install

### 例. 2 raspberry pi（cross）

1. raspbian 用 toolchain (arm-linux-gnueabihf) をインストール
2. 実機上の /opt/vc を開発環境のどこかにコピー 

    $ ./configure --prefix=/path/to/install --host=arm-linux-gnueabihf --enable-png --enable-jpeg --enable-freetype --with-includes=/path/to/vc/include:/path/to/vc/include/interface/vcos/pthreads:/path/to/otherlibs/include:/path/to/otherlibs/include/freetype2
    $ make
    $ make install

ライセンス
----------

### ugles2

<pre>
ugles2

The MIT License

Copyright (c) 2013 @uwitty

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
</pre>

### sample/platform/mesa_x.c

http://code.google.com/p/opengles-book-samples/ の、Linux 向けコードをベースにしています。

### sample/platform/raspberrypi.c

raspbian 実機環境の /opt/vc/src/hello_pi/hello_triangle/triangle.c をベースにしています。

関連情報
--------

1. [opengles-book-samples](http://code.google.com/p/opengles-book-samples/ "opengles-book-samples")
2. /opt/vc/src/hello_pi/hello_triangle (raspbian の実機上のパス)

