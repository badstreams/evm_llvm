//===- Support/Tree.h - Generic n-way tree structure ------------*- C++ -*-===//
//
// This class defines a generic N way tree node structure.  The tree structure
// is immutable after creation, but the payload contained within it is not.
//
//===----------------------------------------------------------------------===//

#ifndef SUPPORT_TREE_H
#define SUPPORT_TREE_H

#include <vector>

template<class ConcreteTreeNode, class Payload>
class Tree {
  std::vector<ConcreteTreeNode*> Children;        // This nodes children, if any
  ConcreteTreeNode              *Parent;          // Parent of this node...
  Payload                        Data;            // Data held in this node...

protected:
  void setChildren(const std::vector<ConcreteTreeNode*> &children) {
    Children = children;
  }
public:
  inline Tree(ConcreteTreeNode *parent) : Parent(parent) {}
  inline Tree(const std::vector<ConcreteTreeNode*> &children,
              ConcreteTreeNode *par) : Children(children), Parent(par) {}

  inline Tree(const std::vector<ConcreteTreeNode*> &children,
              ConcreteTreeNode *par, const Payload &data) 
    : Children(children), Parent(parent), Data(data) {}

  // Tree dtor - Free all children
  inline ~Tree() {
    for (unsigned i = Children.size(); i > 0; --i)
      delete Children[i-1];
  }

  // Tree manipulation/walking routines...
  inline ConcreteTreeNode *getParent() const { return Parent; }
  inline unsigned getNumChildren() const { return Children.size(); }
  inline ConcreteTreeNode *getChild(unsigned i) const {
    assert(i < Children.size() && "Tree::getChild with index out of range!");
    return Children[i];
  }

  // Payload access...
  inline Payload &getTreeData() { return Data; }
  inline const Payload &getTreeData() const { return Data; }
};


#endif
