#include "exchangeMatching.h"

int main(int argc, char * argv[]) {
  int threadpoolsize = 0;
  if (argc > 2) {
    stringstream s(argv[2]);
    s >> threadpoolsize;
    cout << "The size of thread pool is: " << threadpoolsize << endl;
  }
  try {
    Query_funcs q;
    Server s;
    ExchangeMatching exchangeMatching(q, s);
    if (threadpoolsize != 0) {
      exchangeMatching.threadpoolsize = threadpoolsize;
    }
    else {
      exchangeMatching.threadpoolsize = 1;
    }
    exchangeMatching.run();
  }
  catch (const std::exception & e) {
    cerr << e.what() << endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}