# RockDB porting to KV-SSD

# Build Test example

## build Samsung KVSSD

### build emulator (environment without actual device)

```bash
	# build kvapi library
	export PRJ_HOME=$(pwd)
	export KVSSD_HOME=$PRJ_HOME/KVSSD-1.2.0/PDK/core
	$KVSSD_HOME/tools/install_deps.sh # install kvapi dependency
	mkdir $KVSSD_HOME/build
	cd $KVSSD_HOME/build
	cmake -DWITH_EMU=ON $KVSSD_HOME
	make -j4

	# copy libkvapi.so
	mkdir $PRJ_HOME/libs
	cp $KVSSD_HOME/build/libkvapi.so $PRJ_HOME/libs/
```

## build rockskv library

```bash
	make
```

```

rockskv
