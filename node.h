#ifndef NODE_H
#define NODE_H

template <typename TK>
struct Node {
    // array de keys
    TK* keys;
    // array de punteros a hijos
    Node** children;
    // cantidad de keys
    int count;
    // indicador de nodo hoja
    bool leaf;

    Node() : keys(nullptr), children(nullptr), count(0), leaf(false) {}

    explicit Node(const int& M) {
        keys = new TK[M - 1]();
        children = new Node<TK>*[M]();
        count = 0;
        leaf = true;
    }

    void killSelf() {
        delete[] this->keys;
        if (!leaf) {
            for (int i = 0; i <= count; ++i)
                this->children[i]->killSelf();
        }
        delete[] this->children;
        delete this;
    }
};

#endif