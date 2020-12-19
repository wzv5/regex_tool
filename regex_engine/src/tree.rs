#![allow(dead_code)]

use std::cell::RefCell;
use std::rc::{Rc, Weak};

#[derive(Debug)]
pub struct TreeNode<T> {
    content: RefCell<T>,
    children: RefCell<Vec<Rc<TreeNode<T>>>>,
    parent: Option<Weak<TreeNode<T>>>,
}

impl<T> TreeNode<T> {
    pub fn new(content: T) -> Rc<Self> {
        Rc::new(Self::new_inner(content))
    }

    fn new_inner(content: T) -> Self {
        Self {
            content: RefCell::new(content),
            children: Default::default(),
            parent: None,
        }
    }

    pub fn push(self: &Rc<Self>, content: T) -> Rc<Self> {
        let mut item = TreeNode::new_inner(content);
        item.parent = Some(Rc::downgrade(self));
        let item = Rc::new(item);
        self.children.borrow_mut().push(item.clone());
        item
    }

    pub fn parent(&self) -> Option<Rc<Self>> {
        self.parent.as_ref().and_then(|p| p.upgrade())
    }

    pub fn root(self: &Rc<Self>) -> Rc<Self> {
        let mut root = self.clone();
        while let Some(p) = root.parent.as_ref().and_then(|p| p.upgrade()) {
            root = p;
        }
        root
    }

    pub fn content(&self) -> std::cell::Ref<T> {
        self.content.borrow()
    }

    pub fn content_mut(&self) -> std::cell::RefMut<T> {
        self.content.borrow_mut()
    }

    pub fn set_content(&self, content: T) {
        self.content.replace(content);
    }

    pub fn children(&self) -> std::cell::Ref<Vec<Rc<Self>>> {
        self.children.borrow()
    }

    pub fn children_mut(&self) -> std::cell::RefMut<Vec<Rc<Self>>> {
        self.children.borrow_mut()
    }
}

pub type Tree<T> = Rc<TreeNode<T>>;
