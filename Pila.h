#ifndef PILA_H
#define PILA_H

#include <stdexcept>

template <typename T>
class Pila {
    private:
        struct Node {
            T data;
            Node* next;
            Node() : data(T()), next(nullptr) {}
            explicit Node(const T& _data) : data(_data), next(nullptr) {}
        };
    private:
        Node* head;
    public:
        Pila() : head(nullptr) {}

        ~Pila() {
            while (head != nullptr) {
                Node* temp = head;
                head = head->next;
                delete temp;
            }
        }

        void push(const T& data) {
            Node* nuevo = new Node(data);
            nuevo->next =  head;
            head = nuevo;
        }

        T pop() {
            if (is_empty())
                throw std::runtime_error("Pila vacía");
            Node* temp = head;
            T data =  temp->data;
            head = head->next;
            delete temp;
            return data;
        }

        T& top() {
            if (is_empty())
                throw std::runtime_error("Pila vacía");
            return head->data;
        }

        const T& top() const {
            if (is_empty())
                throw std::runtime_error("Pila vacía");
            return head->data;
        }

        bool is_empty() const {
            return head == nullptr;
        }

};

#endif