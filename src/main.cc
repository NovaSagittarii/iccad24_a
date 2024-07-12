#include <cmath>
#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <bits/stdc++.h>

std::vector<std::chrono::_V2::system_clock::time_point> times;
void StartClock() {
  times.push_back(std::chrono::high_resolution_clock::now());
}
int EndClock() {
  auto t0_ = times.back();
  times.pop_back();
  using std::chrono::high_resolution_clock;
  using std::chrono::duration_cast;
  using std::chrono::duration;
  using std::chrono::milliseconds;
  auto t1 = std::chrono::high_resolution_clock::now();
  auto ms_int = duration_cast<milliseconds>(t1 - t0_);
  return ms_int.count();
}
std::map<std::string, int> kTiming;
std::map<std::string, int> kInvocation;

#include <stdlib.h>
#include <stdio.h>
#include <fstream>
std::string cost_function_path, library_path, input_netlist_path, output_path;
double Evaluate() {
  const std::string cost_output_path = "cost_eval.txt";
  std::string cmd = cost_function_path
    + " -library " + library_path
    + " -netlist " + output_path
    + " -output " + cost_output_path
    + " >/dev/null";
  
  StartClock();
  int result = std::system(cmd.c_str());
  kTiming["cost_eval"] += EndClock();
  ++kInvocation["cost_eval"];

  if (WIFSIGNALED(result)) { // https://stackoverflow.com/a/3771792
    printf("Exited with signal %d\n", WTERMSIG(result));
    exit(1);
  }
  std::ifstream fin("cost_eval.txt");
  std::string dummy;
  double cost = 1e300;
  fin >> dummy >> dummy >> cost;
  fin.close();
  return cost;
}

double AcceptProbability(double E, double Ep, double T) {
  if (Ep < E) return 1;
  else return std::exp(-(Ep - E) / T);
}

void Write(std::vector<std::string> header, std::vector<std::string> body) {
  StartClock();
  
  std::ofstream fout(output_path);
  for (auto& line : header) fout << line << "\n";
  for (auto& line : body) fout << line << "\n";
  fout << "endmodule\n";
  fout.close();

  kTiming["write"] += EndClock();
  ++kInvocation["write"];
}

int32_t main(int argc, char** argv) {
  std::srand(1);
  std::string* write_to = nullptr;
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "-library") write_to = &library_path;
    else if (arg == "-cost_function") write_to = &cost_function_path;
    else if (arg == "-netlist") write_to = &input_netlist_path;
    else if (arg == "-output") write_to = &output_path;
    else {
      if (write_to) *write_to = arg;
      write_to = nullptr;
    }
  }

  #define THROW_IF_EMPTY(a, err) if (a.empty()) { \
    std::cout << err << "\n"; \
    return 1; \
  }
  THROW_IF_EMPTY(cost_function_path, "Missing cost function.")
  THROW_IF_EMPTY(library_path, "Missing library.")
  THROW_IF_EMPTY(input_netlist_path, "Missing netlist.")
  THROW_IF_EMPTY(output_path, "Missing output.")

  std::vector<std::string> header, body, best_body;
  std::vector<int> body_idx;
  std::vector<int> body_variants;

  std::ifstream fin(input_netlist_path);
  std::string line;
  while (std::getline(fin, line)) {
    const std::set<std::string> kGates {
      "or", "nor", "and", "nand", "xor", "xnor", "buf", "not"
    };
    std::map<std::string, int> kGateVariants {
      {"and", 8},
      {"nand", 8},
      {"or", 8},
      {"nor", 8},
      {"xor", 6},
      {"xnor", 6},
      {"buf", 9},
      {"not", 9},
    };
    std::string word;
    std::istringstream in(line);
    in >> word;
    char type = word.back();
    for (int i = 0; i < 2 && !word.empty(); ++i) word.pop_back();
    if (kGates.count(word)) {
      line = "\t" + word + "_" + type + " ";
      body_idx.push_back(word.size() + 1 + 1); // the index to edit
      body_variants.push_back(kGateVariants[word]);
      
      std::vector<std::string> words;
      while (in >> word) words.push_back(word);
      // time to do the swap, 2 4 6
      // if (words.size() > 6) std::swap(words[2], words[6]);
      // else std::swap(words[2], words[4]);

      for (auto &word : words) line += word + " ";
      body.push_back(line);
    } else {
      header.push_back(line);
    }
  }
  header.pop_back(); // endmodule

  const std::vector<std::string> original_body = body;
  double E_low = 1e300;

  std::vector<double> uphill_deltas;
  std::map<double, std::vector<double>> cost_deltas;

  for (int tn = 0; tn < 3; ++tn) {
    body = original_body;
    Write(header, body);
    double E = Evaluate();

    const int kMaxIter = 3000;
    const double kMaxTemp = 1e-2;
    const double kMinTemp = 1e-6;
    double T;
    double T1 = std::abs(6.1445297e-1 / std::log(0.8));

    for (int i = 0; i < kMaxIter; ++i) {
      StartClock();

      // T = (std::exp(1 - i / (double)kMaxIter) - 1) / (std::exp(1.0) - 1) * (kMaxTemp - kMinTemp) + kMinTemp;
      T = T1 * 0.2 / i;
      if (i == 0) T = T1;
      else if (i < 500) T /= 100;

      if (i % 10 == 0) {
        std::cout << std::fixed
          << "epoch=" << std::setw(5) << tn
          << " iter=" << std::setw(5) << i
          << " T=" << std::setw(10) << std::setprecision(7) << T
          << std::scientific
          << " curr = " << std::setw(10) << E
          << " best = " << std::setw(10) << E_low << std::endl;
        if (i % 1000 != 0) std::cout << "\u001b[1F\u001b[1K";
      }
      
      std::vector<std::tuple<int, char, char>> changes;
      if (i < 10) {
        for (int idx = 0; idx < body.size(); ++idx) {
          char old_variant = body[idx][body_idx[idx]];
          char new_variant = (std::rand() % body_variants[idx]) + '1';
          changes.push_back({idx, old_variant, new_variant});
        }
      }
      for (int j = 0; j < 1; ++j) {
        int idx = std::rand() % body.size();
        char old_variant = body[idx][body_idx[idx]];
        char new_variant = (std::rand() % body_variants[idx]) + '1';
        changes.push_back({idx, old_variant, new_variant});
      }
      // apply update
      for (auto [idx, _, v] : changes) body[idx][body_idx[idx]] = v;
      Write(header, body);
      double Ep = Evaluate();

      // average uphill cost
      // if (Ep > E) uphill_deltas.push_back(Ep - E);
      // cost delta
      // cost_deltas[std::round(std::log2(T))].push_back(Ep - E);

      if (AcceptProbability(E, Ep, T) * RAND_MAX >= std::rand()) {
        // keep it
        E = Ep;
        if (E < E_low) {
          E_low = E;
          best_body = body;
        }
      } else {
        // discard the change
        for (auto [idx, revert, _] : changes) body[idx][body_idx[idx]] = revert;
      }

      kTiming["iter"] += EndClock();
      ++kInvocation["iter"];
    }
  }

  Write(header, best_body);
  std::cout << std::endl;
  std::cout << "best = " << E_low << std::endl;

  // double tot = 0;
  // for (auto x : uphill_deltas) tot += x;
  // std::cout << "avg uphill = " << (tot/uphill_deltas.size()) << "\n";

  // for (auto [k, a] : cost_deltas) {
  //   tot = 0;
  //   for (auto x : a) tot += x;
  //   std::cout << "bucket=" << k << " cost_avg= " << (tot/a.size()) << "\n";
  // }

  std::cout << std::endl;
  for (auto [k, tot] : kTiming) {
    int n = kInvocation[k];
    std::cout << std::setw(20) << k 
      << " n=" << std::setw(5) << kInvocation[k]
      << " tot=" << std::setw(10) << tot << "ms"
      << " avg=" << std::setw(10) << (tot/n) << "ms" << "\n";
  }

  return 0;
}