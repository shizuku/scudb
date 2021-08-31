/***********************
 * @file: main.cpp
 * @author: shizuku
 * @date: 2021/8/31
 ***********************/
#include <iostream>

#include "p0_starter.h"

template<class T>
std::ostream& operator<<(std::ostream& o, const scudb::Matrix<T>& m) {
  auto r = m.GetRowCount();
  auto c = m.GetColumnCount();
  for (int i = 0; i < r; i++) {
    for (int j = 0; j < c; j++) {
      o << m.GetElement(i, j) << "\t";
    }
    o << "\n";
  }
  return o;
}

int main() {
  auto a = std::make_unique<scudb::RowMatrix<int>>(3, 4);
  a->FillFrom({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12});
  std::cout << *a << std::endl;

  auto b = std::make_unique<scudb::RowMatrix<int>>(4, 3);
  b->FillFrom({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12});
  std::cout << *b << std::endl;

  auto c = std::make_unique<scudb::RowMatrix<int>>(3, 4);
  c->FillFrom({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12});
  std::cout << *c << std::endl;

  auto d = std::make_unique<scudb::RowMatrix<int>>(3, 3);
  d->FillFrom({1, 2, 3, 4, 5, 6, 7, 8, 9});
  std::cout << *d << std::endl;

  auto e = scudb::RowMatrixOperations<int>::Add(a.get(), c.get());
  std::cout << *e << std::endl;

  auto f = scudb::RowMatrixOperations<int>::Multiply(a.get(), b.get());
  std::cout << *f << std::endl;

  auto g = scudb::RowMatrixOperations<int>::GEMM(a.get(), b.get(), d.get());
  std::cout << *g << std::endl;
}
