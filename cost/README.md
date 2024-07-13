# Cost function re-implementation
> Reimplemented cost functions (based on decompiler) to speed up cost computation.

## Usage
```sh

```

## Algorithm

### Nets

Nets are characterized as acyclic and each net has a driver and fan-out. The driver may be a gate or primary-input.

### Module name encoding

The module name is encoded as a uint32 array offset by 1234567 (output is 1234567 higher than actual value) which should be reinterpreted as a char array (C string). The C string consists of 3 doubles (as string) separated by underscore.

### Timing

### Power Area

### Dynamic Power

### Power Domain Crossings
