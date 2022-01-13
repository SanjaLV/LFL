# LFL 

LFL - Lielās Futbola Līgas.

## Building from source

Run cmake/make from root directory

! System sqlite3 is required

```bash
cmake -B Build
make -C Build -j8 -l8
```

## Building documentation

Doxygen in required

```bash
sudo apt-get install doxygen
```

Run doxygen from root directory & open html/index.html

```bash
doxygen .doxygen.conf
firefox html/index.html
```

## Using lfl

By default lfl executable is inside root build directory and usage is following

```bash
Usage: lfl [options] file
Options:
	--help              Display this information.
	--single <file>     Process single XML file.
	--dir <directory>   Process all XML files in given directory.
	--generate <file>   Generate statistics to given file.
	--max-player <N>    Truncate generated tables after N-th player.
```

Attention lfl processes command in a chain so it possible so

!! --max-player will NOT change previous --generation behavior
!v It is possible to chain multiple commands together like:
```bash
./lfl --max-player 25 --dir BPL_winter --generate bpl_half.html \
      --dir  BPL_summer --generate bpl.html
```
