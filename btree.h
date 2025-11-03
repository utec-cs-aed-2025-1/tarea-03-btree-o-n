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


// para permitir que la key se pueda convertir a string
template <typename TK>
std::string keyToString(const TK& key) {
    if constexpr (std::is_same_v<TK, std::string>)
    return key;
    else if constexpr (std::is_convertible_v<TK, std::string>)
    return std::string(key);
    else if constexpr (std::is_arithmetic_v<TK>) {
        std::ostringstream oss;
        oss << key;
        return oss.str();
    } else {
        std::ostringstream oss;
        oss << key;  // requiere operator<< sobrecargado en la clase TK
        return oss.str();
    }
}


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




    bool search(const TK &key) const { // asumiendo que los punteros de children estan inicializados con nullptr
        Node<TK> *current = root;
        while (current != nullptr) {
            for (int i = 0; i < current->count; ++i) {
                if (key < current->keys[i]) {
                    current = current->children[i];
                    break;
                }
                if (current->keys[i] < key) {
                    if (i + 1 == current->count) {
                        current = current->children[current->count];
                        break;
                    }
                    continue;
                }
                return true;
            }
        }
        return false;
    }

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
    string toString(const std::string &sep = " ") const {
        string result;
        toString(root, result, sep);
        return result;
    } // recorrido inorder
    vector<TK> rangeSearch(const TK& begin,const TK& end) {
        vector<TK> result;
        if (root == nullptr || begin > end)
            return result;
        rangeSearchRec(root, begin, end, result);
        return result;
    }

    TK minKey();  // minimo valor de la llave en el arbol
    TK maxKey();  // maximo valor de la llave en el arbol
    void clear() {
        if (root != nullptr) {
            root->killSelf();
            root = nullptr;
            n = 0;
        }
    }// eliminar todos lo elementos del arbol
    const int& size() const {
        return n;
    }// retorna el total de elementos insertados

    // Construya un árbol B a partir de un vector de elementos ordenados
    static BTree* build_from_ordered_vector(std::vector<TK> &elements, const int& M) {
        BTree<TK>* btree = new BTree(M);
        Pair<Node<TK>*, int>* basePromoted = nullptr;
        if (elements.size() < M) {
            btree->root =
                    build_from_ordered_vector_recursivo(elements, basePromoted, elements.size() + 1, M);
        } else {
            basePromoted = new Pair<Node<TK>*, int>[elements.size() + 1];
            for (int i = 0; i <= elements.size(); ++i) {
                basePromoted[i].first = nullptr;
                basePromoted[i].second = i;
            }

            btree->root =
                    build_from_ordered_vector_recursivo(elements, basePromoted, elements.size() + 1, M);
        }
        return btree;
    }

    // Verifique las propiedades de un árbol B
    bool check_properties() const {
        return check_properties_rec(root).valid;
    }
    bool empty() const {
        return root == nullptr;
    }
private:

    // Construye el camino desde la raíz hasta la posición donde se encuentra o debería insertarse la key.
    bool findPathToKey(const TK &key,
                       Pila<Pair<Node<TK> *, int>> &pila) const { // todos los children deben de estar con nullptr si es hoja
        Node<TK> *current = root;
        while (current != nullptr) {
            for (int i = 0; i < current->count; ++i) {
                if (key < current->keys[i]) {
                    pila.push({current, i});
                    current = current->children[i];
                    break;
                }
                if (current->keys[i] < key) {
                    if (i + 1 == current->count) {
                        pila.push({current, current->count});
                        current = current->children[current->count];
                        break;
                    }
                    continue;
                }
                pila.push({current, i});
                return true;
            }
        }
        return false;
    }

    // se usa para insertar un valor con su hijo derecho en un nodo que tiene espacio
    void insertIntoNode(Node<TK> *const &node, const int &index, const TK &value,
                        Node<TK> *const &rightOfValue) {
        node->children[node->count + 1] = node->children[node->count];
        for (int i = node->count; i > index; --i) {
            node->keys[i] = node->keys[i - 1];
            node->children[i] = node->children[i - 1];
        }
        node->keys[index] = value;
        node->children[index + 1] = rightOfValue;
        ++node->count;
    }

    Pair<Node<TK>*, TK> split(Node<TK> *const &node, const int &index, const TK &value, Node<TK> *const &rightOfValue) {
        int medianIndex = (M - 1) / 2;
        TK median = TK();
        Node<TK> *rightNode = new Node<TK>(M);

        if (index < medianIndex) {
            // valor mediano
            median = node->keys[medianIndex - 1];
            node->keys[medianIndex - 1] = TK();

            // actualizando nodo derecho del split
            for (int i = medianIndex, j = 0; i < node->count; ++i, ++j) {
                rightNode->keys[j] = node->keys[i];
                node->keys[i] = TK();
                rightNode->children[j] = node->children[i];
                node->children[i] = nullptr;
            }
            rightNode->children[node->count - medianIndex] = node->children[node->count];
            node->children[node->count] = nullptr;

            rightNode->count = node->count - medianIndex;
            rightNode->leaf = node->leaf;

            // actualizando nodo izquierdo del split
            node->children[medianIndex] = node->children[medianIndex - 1];
            for (int i = medianIndex - 1; i > index; --i) {
                node->keys[i] = node->keys[i - 1];
                node->children[i] = node->children[i - 1];
            }
            node->keys[index] = value;
            node->children[index + 1] = rightOfValue;

            node->count = medianIndex;

        } else if (medianIndex < index) {
            // valor mediano
            median = node->keys[medianIndex];
            node->keys[medianIndex] = TK();

            // actualizando nodo derecho del split
            for (int i = medianIndex + 1, j = 0; i < index; ++i, ++j) {
                rightNode->keys[j] = node->keys[i];
                node->keys[i] = TK();
                rightNode->children[j] = node->children[i];
                node->children[i] = nullptr;
            }
            rightNode->children[index - medianIndex - 1] = node->children[index];
            node->children[index] = nullptr;

            rightNode->keys[index - medianIndex - 1] = value;
            node->children[index] = rightOfValue;

            for (int i = index, j = index - medianIndex; i < node->count; ++i, ++j) {
                rightNode->keys[j] = node->keys[i];
                node->keys[i] = TK();
                rightNode->children[j] = node->children[i];
                node->children[i] = nullptr;
            }
            rightNode->children[node->count - medianIndex] = node->children[node->count];
            node->children[node->count] = nullptr;

            rightNode->count = node->count - medianIndex;
            rightNode->leaf = node->leaf;

            // actualizando nodo izquierdo del split
            node->count = medianIndex;

        } else {
            // valor mediano
            median = value;

            // actualizando nodo derecho del split
            rightNode->children[0] = rightOfValue;
            for (int i = medianIndex, j = 0; i < node->count; ++i, ++j) {
                rightNode->keys[j] = node->keys[i];
                node->keys[i] = TK();
                rightNode->children[j + 1] = node->children[i + 1];
                node->children[i + 1] = nullptr;
            }
            rightNode->count = node->count - medianIndex;
            rightNode->leaf = node->leaf;

            // actualizando nodo izquierdo del split
            node->count = medianIndex;
        }

        return {rightNode, median};
    }


    void removeKeyFromLeaf(Node<TK>* const& node, const int& index) {
        for (int i = index; i < node->count - 1; ++i) {
            node->keys[i] = node->keys[i + 1];
        }
        node->keys[node->count - 1] = TK();
        --node->count;
    }

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

    // --- sucesor
    // Recibe una pila con el camino desde la raíz hasta la clave buscada,
    // incluyendo el nodo y la posición donde se encontró la key.
    Pair<TK, int> successor(Pila<Pair<Node<TK>*, int>>& pila) const {
        if (pila.is_empty())
            throw std::runtime_error("No existe esta key");

        Node<TK>* current = pila.top().first;
        int index = pila.top().second;
        TK key = current->keys[index];

        if (current->leaf) { // es hoja
            if (index + 1 == current->count) { // esta en el extremo
                while (current != root && index + 1 == current->count) {
                    pila.pop();
                    current = pila.top().first;
                    index = pila.top().second;
                }
                if (index == current->count)
                    return {key, index}; // es el maximo, no hay sucesor
                return {current->keys[index], index};  // el sucesor está en un ancestro
            }
            return {current->keys[index + 1], index + 1}; // el sucesor es la siguiente key

        } else {
            current = current->children[index + 1]; // bajar al hijo derecho de la key actual
            pila.top().second = index + 1; // arreglar posicion de busqueda
            pila.push({current, 0});

            // buscar el mínimo en ese subárbol
            while (!current->leaf) {
                current = current->children[0];
                pila.push({current, 0});
            }
            return {current->keys[0], 0};
        }
    }


    void toString(Node<TK>* const& node, std::string& result, const std::string& sep) const {
        if (node == nullptr)
            return;

        for (int i = 0; i < node->count; ++i) {
            toString(node->children[i], result, sep);
            if (!result.empty())
                result += sep;
            result += keyToString(node->keys[i]);
        }
        toString(node->children[node->count], result, sep);
    }

    void rangeSearchRec(Node<TK>* node, const TK& begin, const TK& end, std::vector<TK>& result) const {
        if (node == nullptr) return;

        int i = 0;

        while (i < node->count && node->keys[i] < begin) {
            if (!node->leaf)
                rangeSearchRec(node->children[i + 1], begin, end, result);
            ++i;
        }

        for (; i < node->count && node->keys[i] <= end; ++i) {
            if (!node->leaf)
                rangeSearchRec(node->children[i], begin, end, result);

            if (node->keys[i] >= begin && node->keys[i] <= end)
                result.push_back(node->keys[i]);
        }

        if (!node->leaf)
            rangeSearchRec(node->children[i], begin, end, result);
    }


    // promoted tiene los punteros a los hijos y los indices de los elementos que suben(tiene tamaño size = numero de hijos)
    static Node<TK>* build_from_ordered_vector_recursivo(const std::vector<TK>& elements,
                                                         Pair<Node<TK>*, int>* const&  promoted,
                                                         int size, int M) {
        if (size - 1 < M) { // no se puede dividir, ahi queda
            Node<TK>* root = new Node<TK>(M);

            if (promoted == nullptr) {
                // caso root hoja
                for (int  i = 0; i < elements.size(); ++i) {
                    root->keys[i] = elements[i];
                    ++root->count;
                }
                root->leaf = true;
            } else {
                // caso root no hoja
                for (int  i = 0; i < size - 1; ++i) {
                    root->keys[i] = elements[promoted[i].second];
                    root->children[i] = promoted[i].first;
                    ++root->count;
                }
                root->children[root->count] = promoted[size - 1].first;
                root->leaf = false;
                delete[] promoted;
            }
            return root;

        } else if (promoted != nullptr) {
            // se puede dividir
            int nextLevelSize = (size - 1 + 1 + M - 1) / M;
            Pair<Node<TK>*, int>* nextPromoted = new Pair<Node<TK>*, int>[nextLevelSize];

            int t = 0; // indice de nextPromoted
            int i = 0; // indice de Promoted
            for (; t < nextLevelSize; ++t) {
                Node<TK>* newNode = new Node<TK>(M); // nuevo nodo

                int minDegree = (M % 2 == 0) ? M / 2 : (M + 1) / 2;
                int minKeys = minDegree - 1;

                int keyCount = 0;
                if (size - 1 - i > M -1 && size - 1- i < M + minKeys) { // para respetar los cantidades dos ultimos nodos
                    keyCount = minKeys;
                } else if (size - 1 -i > M - 1)
                    keyCount = M - 1;
                else
                    keyCount = size - 1 -i;

                for (int j = 0; j < keyCount; ++j, ++i) { // j es el índice del nodo creado
                    newNode->keys[j] = elements[promoted[i].second];
                    newNode->children[j] = promoted[i].first;
                    ++newNode->count;
                }
                newNode->children[newNode->count] = promoted[i].first;

                if (promoted[0].first != nullptr)
                    newNode->leaf = false;
                else
                    newNode->leaf = true;

                nextPromoted[t].first = newNode; // almacenar puntero hijo
                nextPromoted[t].second = promoted[i].second; // almacenar posicion de padre derecho (en el caso extremo es basura)
                ++i;
            }
            delete[] promoted;
            return build_from_ordered_vector_recursivo(elements, nextPromoted, nextLevelSize, M);
        } else {
            throw std::runtime_error("Promoted no valido");
        }
    }



    // -------------------- check_properties por nodo ---------------

    //1- cada nodo debe tener al menos M/2
    //2- garantizar que las hojas esten al mismo nivel
    // cada nodo debe tener count+1 hijos
    //3- los elementos en el nodo deben estar ordenados
    //4- dado un elemento en un nodo interno:
    // - los elemenos del subarbol izquierdo son menores
    // - los elemenos del subarbol derecho son mayores

    struct SubtreeProperties {
        bool valid;
        int height;
        TK minKey; // minima key del subarbol formado por el nodo
        TK maxKey; // maxima key del subarbol formado por el nodo
    };

    int minDegree = (M % 2 == 0) ? M / 2 : (M + 1) / 2;
    int minKeys = minDegree - 1;

    SubtreeProperties check_properties_rec(Node<TK>* const& node) const {

        if (node == nullptr) {
            return {true, -1, TK(), TK()};
        }

        if (node == root && node->count <= 0) // tiene que tener al menos una llave si es raiz
            return {false, -1, TK(), TK()};

        if (node != root && node->count < minKeys) // minimo de llaves
            return {false, -1, TK(), TK()};

        if (node->count > M - 1) // maximo de llaves
            return {false, -1, TK(), TK()};

        int prevSubtreeHeight = 0;
        TK prevKey = TK();

        TK maxKey = TK(); // maxima llave del subarbol formado por el nodo
        TK minKey = TK(); // minima llave del subarbol formado por el nodo
        int height = 0; // altura del subarbol formado por el nodo

        for (int i = 0; i < node->count; ++ i) {
            SubtreeProperties leftChildProps = check_properties_rec(node->children[i]);

            if (!leftChildProps.valid) // el hijo izquierdo debe de ser válido
                return {false, -1, TK(), TK()};

            if (i == 0) {
                prevKey = node->keys[i];

                if (!node->leaf && node->keys[i] <= leftChildProps.maxKey) // la llave actual debe ser mayor a la maxima llave del subarbol de su hijo izquierdo
                    return {false, -1, TK(), TK()};
                prevSubtreeHeight = leftChildProps.height;

                // minima key del subarbol formado por el del nodo
                if (!node->leaf)
                    minKey = leftChildProps.minKey;
                else
                    minKey = node->keys[i];
            } else {
                if (node->keys[i] <= prevKey) // las llaves deben de estar ordenadas
                    return {false, -1, TK(), TK()};
                if (!node->leaf) {
                    if (prevKey >= leftChildProps.minKey) // la llave anterior debe ser menor a la minima llave del subarbol de su hijo derecho
                        return {false, -1, TK(), TK()};
                    if (node->keys[i] <= leftChildProps.maxKey) // la llave actual debe ser mayor a la maxima llave del subarbol de su hijo izquierdo
                        return {false, -1, TK(), TK()};
                }
                // el hijo izquierdo de la llave actual debe de tener la misma altura que el hijo izquierdo de la llave anterior
                if (prevSubtreeHeight != leftChildProps.height) // esto asegura tambien que las hojas esten al mismo nivel
                    return {false, -1, TK(), TK()};

                prevKey = node->keys[i];
                prevSubtreeHeight = leftChildProps.height;
            }

            if (i + 1 == node->count) {
                SubtreeProperties rightChildProps = check_properties_rec(node->children[node->count]);

                if (!rightChildProps.valid) // el hijo derecho debe de ser válido
                    return {false, -1, TK(), TK()};

                if (!node->leaf && node->keys[i] >= rightChildProps.minKey) // la llabe actual debe ser menor que la minima llave del subarbol de su hijo derecho
                    return {false, -1, TK(), TK()};

                // el hijo izquierdo de la llave actual debe de tener la misma altura que el hijo derecho
                if (leftChildProps.height != rightChildProps.height)
                    return {false, -1, TK(), TK()};

                // maxima key del subarbol formado por el del nodo
                if (!node->leaf)
                    maxKey = rightChildProps.maxKey;
                else
                    maxKey = node->keys[i];
                // altura del nodo actual
                height = rightChildProps.height + 1;
            }
        }

        return {true, height, minKey, maxKey};
    }


public:
    ~BTree() {
        clear();
    }

};

#endif
