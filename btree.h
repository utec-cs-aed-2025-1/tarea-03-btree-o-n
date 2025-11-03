#ifndef BTree_H
#define BTree_H
#include <vector>
#include <string>
#include <stdexcept>
#include <sstream>
#include <type_traits>

#include "node.h"
#include "Pila.h"
#include "Pair.h"

template <typename TK>
class BTree {
private:
  Node<TK>* root;
  int M;  // grado u orden del arbol
  int n; // total de elementos en el arbol 

public:

    explicit BTree(const int& M_) : root(nullptr), M(M_), n(0) {
        if (M < 3)
            throw std::out_of_range("El grado del arbol debe de ser mínimo 3");
    }




    bool search(TK key);//indica si se encuentra o no un elemento
    void insert(TK key);//inserta un elemento
    void remove(TK key);//elimina un elemento
    int height();//altura del arbol. Considerar altura 0 para arbol vacio
    string toString(const string& sep);  // recorrido inorder
    vector<TK> rangeSearch(TK begin, TK end);

    TK minKey();  // minimo valor de la llave en el arbol
    TK maxKey();  // maximo valor de la llave en el arbol
    void clear(); // eliminar todos lo elementos del arbol
    int size(); // retorna el total de elementos insertados
  
    // Construya un árbol B a partir de un vector de elementos ordenados
    static BTree* build_from_ordered_vector(vector<T> elements);
    // Verifique las propiedades de un árbol B
    bool check_properties();

private:


public:
    ~BTree() {
        clear();
    }

};

#endif
