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

    void insert(const TK &key) {
        if (root == nullptr) {
            root = new Node<TK>(M);
            root->keys[0] = key;
            root->count = 1;
            ++n;
            return;
        }

        Pila<Pair<Node<TK> *, int>> pila; // almacena los pares (puntero al nodo y posicion de busqueda)

        bool existe = findPathToKey(key, pila);
        if (existe)
            return; // ya existe

        TK value = key;
        Node<TK> *rightOfValue = nullptr;
        Node<TK> *leftOfValue = nullptr;

        while (true) {
            if (pila.is_empty() || pila.top().first->count < M - 1) { // caso nodo con espacio
                if (pila.is_empty()) {
                    root = new Node<TK>(M);
                    root->children[0] = leftOfValue;
                    root->leaf = false;
                    insertIntoNode(root, 0, value, rightOfValue);
                } else {
                    insertIntoNode(pila.top().first, pila.top().second, value, rightOfValue);
                }
                break;
            } else {
                Pair<Node<TK> *, TK> result =
                        split(pila.top().first, pila.top().second, value, rightOfValue);
                leftOfValue = pila.top().first;
                rightOfValue = result.first;
                value = result.second;
                pila.pop();
            }
        }
        ++n;
    }

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


    // Aplica una rotación entre el nodo y su hermano (izquierdo o derecho),
    // fromLeft = true -> rotar con el hermano izquierdo
    // fromLeft = false -> rotar con el hermano derecho
    void rotate(Node<TK>* const& node, Node<TK>* const& parent, const int& nodeIndex, bool fromLeft) {
        if (fromLeft) {
            Node<TK>* sibling = parent->children[nodeIndex - 1];

            // insertar el valor de la key padre con el rightmostChild del sibling en el nodo actual
            for (int i = node->count;  i > 0; --i) {
                node->keys[i] = node->keys[i - 1];
                node->children[i + 1] = node->children[i];
            }
            node->keys[0] = parent->keys[nodeIndex - 1];
            node->children[1] = node->children[0];
            node->children[0] = sibling->children[sibling->count]; // rightmostChild del sibling
            sibling->children[sibling->count] = nullptr;
            ++node->count;

            // reemplazar padre key por el antecesor en el hijo izquierdo
            parent->keys[nodeIndex - 1] =  sibling->keys[sibling->count - 1];
            sibling->keys[sibling->count - 1] = TK();
            --sibling->count;
        } else {
            Node<TK>* sibling = parent->children[nodeIndex + 1];

            // insertar el valor de la key padre con el leftmostChild del sibling en el nodo actual
            node->keys[node->count] = parent->keys[nodeIndex];
            node->children[node->count + 1] = sibling->children[0]; // leftmostChild del sibling
            sibling->children[0] = nullptr;
            ++node->count;

            // reemplazar padre key por el sucesor en el hijo derecho
            parent->keys[nodeIndex] = sibling->keys[0];
            // remover la key en la posicion 0 del sibling
            for (int i = 0; i < sibling->count - 1; ++i) {
                sibling->keys[i] = sibling->keys[i + 1];
                sibling->children[i] = sibling->children[i + 1];
            }
            sibling->keys[sibling->count - 1] = TK();
            sibling->children[sibling->count - 1] = sibling->children[sibling->count];
            sibling->children[sibling->count] = nullptr;
            --sibling->count;
        }
    }

    void merge(Node<TK>* const& node, Node<TK>* const& parent, const int& nodeIndex, bool fromLeft) {
        if (fromLeft) {
            Node<TK>* sibling = parent->children[nodeIndex - 1];

            // insertar la key padre en el hermano izquierdo
            sibling->keys[sibling->count] = parent->keys[nodeIndex - 1];
            ++sibling->count;

            // eliminar la key padre del nodo padre
            for (int i = nodeIndex - 1; i < parent->count - 1; ++i) {
                parent->keys[i] = parent->keys[i + 1];
                parent->children[i] = parent->children[i + 1];
            }
            parent->children[parent->count - 1] = parent->children[parent->count];

            parent->keys[parent->count - 1] = TK();
            parent->children[parent->count] = nullptr;
            parent->children[nodeIndex - 1] = sibling; // reconectar hijo izquierdo
            --parent->count;

            // mover keys e hijos del nodo actual al hermano izquierdo
            for (int i = sibling->count, j = 0; j < node->count; ++i, ++j) {
                sibling->keys[i] = node->keys[j];
                node->keys[j] = TK();
                sibling->children[i] = node->children[j];
                node->children[j] = nullptr;
                ++sibling->count;
            }
            sibling->children[sibling->count] = node->children[node->count];
            node->children[node->count] = nullptr;

            // eliminar nodo actual
            delete[] node->keys;
            delete[] node->children;
            delete node;

        } else { // es muy parecido a lo anterior, asi que se puede juntar en uno solo, pero lo dejo así por ahora
            Node<TK> *sibling = parent->children[nodeIndex + 1];

            // insertar la key padre en el nodo actual
            node->keys[node->count] = parent->keys[nodeIndex];
            ++node->count;

            // eliminar la key padre del nodo padre
            for (int i = nodeIndex; i < parent->count - 1; ++i) {
                parent->keys[i] = parent->keys[i + 1];
                parent->children[i] = parent->children[i + 1];
            }
            parent->children[parent->count - 1] = parent->children[parent->count];

            parent->keys[parent->count - 1] = TK();
            parent->children[parent->count] = nullptr;
            parent->children[nodeIndex] = node; // reconectar nodo actual
            --parent->count;

            // mover todos los keys e hijos del hermano derecho al nodo actual
            for (int i = node->count, j = 0; j < sibling->count; ++i, ++j) {
                node->keys[i] = sibling->keys[j];
                sibling->keys[j] = TK();
                node->children[i] = sibling->children[j];
                sibling->children[j] = nullptr;
                ++node->count;
            }
            node->children[node->count] = sibling->children[sibling->count];
            sibling->children[sibling->count] = nullptr;

            // eliminar hermano derecho
            delete[] sibling->keys;
            delete[] sibling->children;
            delete sibling;
        }
    }


public:
    ~BTree() {
        clear();
    }

};

#endif
