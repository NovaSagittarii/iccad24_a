# Cost function re-implementation
> Reimplemented cost functions (based on decompiler) to speed up cost computation.

## Usage
```sh
# from project root directory
cd build; make -f ../Makefile verilog_parser
(cd build; make -f ../Makefile verilog_parser) && ./build/verilog_parser ./design1_map.v
```

With VS Code, you may need to add `${workspaceFolder}/**/include/**` to 
the `includePath` so that IntelliSense works property.

# Cost Function Algorithm

The following sections describes what the cost functions do,
and how to implement them.

## Gates

Gates are characterized by attributes defined by the cell library.
```cpp
this +  8 = std::map<std::string, double> // maps attribute_f name to value
this + 12 = std::map<std::string, int>    // maps attribute_i name to value

```

## Nets

Nets are characterized as acyclic and each net has a driver and fan-out.
The driver may be a gate or primary-input.
Fan-out includes at least one element of gates or primary-output.

```cpp
this +  9 = bool
this + 40 = NET*                          // probably the driver
```

## Module name encoding

The module name is encoded as a uint32 array offset by 1234567
(output is 1234567 higher than actual value)
which should be reinterpreted as a char array (C string).
The C string consists of 3 doubles (as string) separated by underscore,
these are the values of `clock_period`, `area_constraint`,
and `power_constraint`, respectively.

## Timing

- Compute the arrival time `arr_time` and required time `req_time` for each gate.
- You can compute slack from arrival time and required time, use the slack to
  compute worst negative slack `wns` and total negative slack `tns`.

### Arrival Time

Uses the following recursive function.
If the net's driver is from the input port, the arrival time is `0.0`.


### Required Time

## PPA

- Total power is the sum of `attr_leak` (`cell_leakage_power_f`) among all gates.
- Total area is the sum of `attr_area` (`area_f`) among all gates.

### Dynamic Power

## Power Domain Crossings (`cross_pd`)
