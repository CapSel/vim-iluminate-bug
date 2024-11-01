# vim-iluminate-bug
Demo for https://github.com/RRethy/vim-illuminate/issues/204

# compilation
Inside cloned directory run:
```
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -G Ninja
ln -s ./build/compile_commands.json .
```
# where is the problem
problem is visible with function `region`. When you place cursor on definition pressing 'n' works only once and switches to usage. Pressing 'p' continuously switches between definition and usage.
