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

## Benchmarks
Times are per run averaged over 100 runs (1000 runs for design1).

| Design | `cost_estimator_1`       | Ours 1   |
| ------ | ------------------------ | -------- |
| `design1_map_simple.v` | 4.225μs  | 2.710μs  |
| `design5_map_simple.v` | 159.17ms | 103.09ms |
| `design6_map_simple.v` | 372.11ms | 287.79ms |

### Regarding Parsing
Times are per run, averaged over 100 runs (design1_map averaged over 1000 runs)
| Design | `cost_estimator_1` (control) | `cost_estimator_8` (experimental) | % of time in setup, parsing, PPA |
| ---- | --- | --- | --- |
| `design1_map.v` |  3.535 μs |  3.835 μs | 92.18% |
| `design5_map.v` | 144.39 ms | 188.00 ms | 76.56% |
| `design6_map.v` | 335.34 ms | 414.33 ms | 80.94% |

Setup and parsing are the main bottleneck for cost computation.
PPA iterates over all gates and sums properties so it doesn't take very long.

# Cost Function Algorithm

The following sections describes what the cost functions do,
and how to implement them.

## Cells

Cells are characterized by attributes defined by the cell library. 

```cpp
this +   0 = 0LL or 1LL                    // 1LL if 2-input gate
this +   8 = std::map<std::string, double> // maps attribute_f name to value
this +  12 = std::map<std::string, int>    // maps attribute_i name to value
this + 104 = double                        // attr_b
```

## Gates

Gates are instances of cells.

```cpp
this +  0 = int32_t                       // gate_id
this +  8 = std::string                   // instance_name of gate
this + 16 = CELL                          // the cell this is an instance of
this + 24 = std::map<std::string, PIN*>   // ?
this + 36 = ?
this + 40 = ?
this + 44 = ?
this + 48 = std::vector<PIN*>             // input pins ?
this + 72 = PIN                           // output pin ?
this + 96 = NET*                          // output; the net it drives

__int64 create_instance(const std::string *a1, const std::string *a2)
# a1 is the cell_type, a2 is instance_name
```

## Nets

Nets are characterized as acyclic and each net has a driver and fan-out.
The driver may be a gate or primary-input.
Fan-out includes gate(s) and/or primary-output.

```cpp
this +  0 = int32_t                       // net_id
this +  8 = double  [tmp]                 // (inputs) * (cell capacitance)
this + 16 = double  [tmp]                 // slp (static leakage power?)
this + 24 = double                        // set probability
this + 32 = double                        // leakage power coefficient (in dyn_power computation)
this + 32 = int32_t [tmp]                 // size of std::vector<PIN*> + is_output
this + 40 = std::string                   // name
this + 48 = std::vector<PIN*>             // fanout
this + 72 = GATE*                         // driver, the gate that drives the net
                                          // might be the Y pin of the driver gate...
```

### Net+16 computation
Define `fout_size` as the number of fanout pins. Output count for 2.
Then `slp_for_fout_size` is some lookup table for various thresholds.
The value is `fout_size * 0.002 * slp_for_fout_size(fout_size)`.

## Pins

Pins connect gates to nets.

```cpp
this +  0 = ?
this +  8 = std::string                   // the name of the pin
this + 16 = ?
this + 24 = NET*                          // net the pin is connected to (used in set_prob)
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

If the net's driver is from the input port, the arrival time is `0.0`.
Recursively add a gate's `attr_b` to find the arrival time of later nets.

### Required Time

If the net's output is the output port, the arrival time is `clock_period`.
Recursively subtract a gate's `attr_b` to find the required time of earlier nets.

## PPA

- Total power is the sum of `attr_leak` (`cell_leakage_power_f`) among all gates.
- Total area is the sum of `attr_area` (`area_f`) among all gates.

### Dynamic Power

- Compute every net's set probability `p`.
- Compute a power coefficient `q = 2(p)(1-p)` for each net.
- The dynamic power of a gate is the product of output net's `q` and the gate's
  leakage power.

## Power Domain Crossings (`cross_pd`)

The total number of unordered pairs of pins on the same net whose gates
have different power domains.

Computed as, the total number of fanout pins associated with a gate with
different power domain among all gates.

## Gate Capacitance

The capacitance experienced by a gate is `capacitance` times the number of
input pins plus some factor dependent on the number of other gates on the net
that its output drives. See `slp_for_fout_size(int)`.
