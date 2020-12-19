use std::{ops::Range, rc::Rc};

use super::tree::{Tree, TreeNode};
use regex_syntax::ast::Ast;

#[derive(Debug)]
pub struct TreeItem {
    pub title: String,
    pub content: String,
    pub span: Range<u32>,
}

impl TreeItem {
    pub fn new(title: String, content: String, start: usize, end: usize) -> Self {
        Self {
            title,
            content,
            span: Range {
                start: start as _,
                end: end as _,
            },
        }
    }
}

pub fn parse(s: &str, ignore_whitespace: bool) -> anyhow::Result<Tree<TreeItem>> {
    let mut builder = regex_syntax::ast::parse::ParserBuilder::new();
    builder.ignore_whitespace(ignore_whitespace);
    let mut parser = builder.build();
    let ast = parser.parse(s)?;
    let visitor = MyAstVisitor::new();
    let tree = regex_syntax::ast::visit(&ast, visitor)?;
    Ok(tree)
}

struct MyAstVisitor {
    root: Tree<TreeItem>,
    tree: Tree<TreeItem>,
}

impl MyAstVisitor {
    fn new() -> Self {
        let root = TreeNode::new(TreeItem::new("root".into(), String::new(), 0, 0));
        Self {
            tree: root.clone(),
            root,
        }
    }
}

impl regex_syntax::ast::Visitor for MyAstVisitor {
    type Output = Tree<TreeItem>;

    type Err = anyhow::Error;

    fn finish(self) -> Result<Self::Output, Self::Err> {
        merge_literal(&self.root);
        if self.root.children().is_empty() {
            self.root
                .push(TreeItem::new("(空白)".into(), String::new(), 0, 0));
        }
        Ok(self.root.children().first().unwrap().clone())
    }

    fn start(&mut self) {}

    fn visit_pre(&mut self, ast: &Ast) -> Result<(), Self::Err> {
        let (title, content) = match ast {
            Ast::Empty(span) => ("(空白)".to_string(), String::new()),
            Ast::Flags(flags) => (
                "标志".into(),
                format!(
                    "{:?}",
                    flags
                        .flags
                        .items
                        .iter()
                        .map(|i| &i.kind)
                        .collect::<Vec<_>>()
                ),
            ),
            Ast::Literal(literal) => ("字符".into(), literal.c.to_string()),
            Ast::Dot(span) => ("任意字符".into(), String::new()),
            Ast::Assertion(assertion) => ("文本边界".into(), format!("{:?}", assertion.kind)),
            Ast::Class(class) => match class {
                regex_syntax::ast::Class::Unicode(class1) => (
                    "Unicode 类别".into(),
                    format!(
                        "{}{:?}",
                        if class1.negated { "非" } else { "" },
                        class1.kind
                    ),
                ),
                regex_syntax::ast::Class::Perl(class1) => (
                    "Perl 类别".into(),
                    format!(
                        "{}{:?}",
                        if class1.negated { "非" } else { "" },
                        class1.kind
                    ),
                ),
                regex_syntax::ast::Class::Bracketed(class1) => {
                    if class1.negated {
                        ("不在集合中".into(), String::new())
                    } else {
                        ("字符集合".into(), String::new())
                    }
                }
            },
            Ast::Repetition(repetition) => {
                let mode = if repetition.greedy {
                    "贪婪"
                } else {
                    "勉强"
                };
                let s = match &repetition.op.kind {
                    regex_syntax::ast::RepetitionKind::ZeroOrOne => "0次或1次".into(),
                    regex_syntax::ast::RepetitionKind::ZeroOrMore => "0次或多次".into(),
                    regex_syntax::ast::RepetitionKind::OneOrMore => "1次或多次".into(),
                    regex_syntax::ast::RepetitionKind::Range(range) => match range {
                        regex_syntax::ast::RepetitionRange::Exactly(m) => format!("{}次", m),
                        regex_syntax::ast::RepetitionRange::AtLeast(m) => format!("至少{}次", m),
                        regex_syntax::ast::RepetitionRange::Bounded(m, n) => {
                            format!("{}~{}次", m, n)
                        }
                    },
                };
                (format!("{}模式重复", mode), s)
            }
            Ast::Group(group) => match &group.kind {
                regex_syntax::ast::GroupKind::CaptureIndex(i) => {
                    (format!("索引分组<{}>", i), String::new())
                }
                regex_syntax::ast::GroupKind::CaptureName(n) => (
                    format!("命名分组<{}>", n.name),
                    format!("index: {}", n.index),
                ),
                regex_syntax::ast::GroupKind::NonCapturing(f) => (
                    "非捕获分组".into(),
                    format!("{:?}", f.items.iter().map(|i| &i.kind).collect::<Vec<_>>()),
                ),
            },
            Ast::Alternation(alternation) => ("任选其一".into(), String::new()),
            Ast::Concat(concat) => ("顺序匹配".into(), String::new()),
        };
        self.tree = self.tree.push(TreeItem::new(
            title,
            content,
            ast.span().start.offset,
            ast.span().end.offset,
        ));
        Ok(())
    }

    fn visit_post(&mut self, ast: &Ast) -> Result<(), Self::Err> {
        match ast {
            Ast::Empty(span) => {}
            Ast::Flags(flags) => {}
            Ast::Literal(literal) => {}
            Ast::Dot(span) => {}
            Ast::Assertion(assertion) => {}
            Ast::Class(class) => {}
            Ast::Repetition(repetition) => {}
            Ast::Group(group) => {}
            Ast::Alternation(alternation) => {}
            Ast::Concat(concat) => {}
        }
        self.tree = self.tree.parent().unwrap();
        Ok(())
    }

    fn visit_alternation_in(&mut self) -> Result<(), Self::Err> {
        //println!("visit_alternation_in");
        Ok(())
    }

    fn visit_class_set_item_pre(
        &mut self,
        ast: &regex_syntax::ast::ClassSetItem,
    ) -> Result<(), Self::Err> {
        let (title, content) = match ast {
            regex_syntax::ast::ClassSetItem::Empty(_) => ("(空白)".to_string(), String::new()),
            regex_syntax::ast::ClassSetItem::Literal(item) => ("字符".into(), item.c.to_string()),
            regex_syntax::ast::ClassSetItem::Range(item) => (
                "字符范围".into(),
                format!("{}-{}", item.start.c, item.end.c),
            ),
            regex_syntax::ast::ClassSetItem::Ascii(item) => (
                "Ascii 类别".into(),
                format!("{}{:?}", if item.negated { "非" } else { "" }, item.kind),
            ),
            regex_syntax::ast::ClassSetItem::Unicode(item) => (
                "Unicode 类别".into(),
                format!("{}{:?}", if item.negated { "非" } else { "" }, item.kind),
            ),
            regex_syntax::ast::ClassSetItem::Perl(item) => (
                "Perl 类别".into(),
                format!("{}{:?}", if item.negated { "非" } else { "" }, item.kind),
            ),
            regex_syntax::ast::ClassSetItem::Bracketed(item) => (if item.negated {
                ("不在集合中".into(), String::new())
            } else {
                ("字符集合".into(), String::new())
            })
            .into(),
            regex_syntax::ast::ClassSetItem::Union(_) => {
                return Ok(());
            }
        };
        self.tree = self.tree.push(TreeItem::new(
            title,
            content,
            ast.span().start.offset,
            ast.span().end.offset,
        ));
        Ok(())
    }

    fn visit_class_set_item_post(
        &mut self,
        ast: &regex_syntax::ast::ClassSetItem,
    ) -> Result<(), Self::Err> {
        match ast {
            regex_syntax::ast::ClassSetItem::Union(_) => {
                return Ok(());
            }
            _ => {
                self.tree = self.tree.parent().unwrap();
            }
        }

        Ok(())
    }

    fn visit_class_set_binary_op_pre(
        &mut self,
        _ast: &regex_syntax::ast::ClassSetBinaryOp,
    ) -> Result<(), Self::Err> {
        //println!("visit_class_set_binary_op_pre");
        Ok(())
    }

    fn visit_class_set_binary_op_post(
        &mut self,
        _ast: &regex_syntax::ast::ClassSetBinaryOp,
    ) -> Result<(), Self::Err> {
        //println!("visit_class_set_binary_op_post");
        Ok(())
    }

    fn visit_class_set_binary_op_in(
        &mut self,
        _ast: &regex_syntax::ast::ClassSetBinaryOp,
    ) -> Result<(), Self::Err> {
        //println!("visit_class_set_binary_op_in");
        Ok(())
    }
}

fn merge_literal(tree: &Tree<TreeItem>) {
    if tree.content().title == "顺序匹配" {
        let is_text = tree.children().iter().all(|i| i.content().title == "字符");
        if is_text {
            let mut buf = String::new();
            for i in tree.children().iter() {
                buf.push_str(&i.content().content);
            }
            tree.content_mut().title = "普通文本".into();
            tree.content_mut().content = buf;
            tree.children_mut().clear();
        }
    } else if tree.content().title == "任选其一" {
        for i in tree.children_mut().iter_mut() {
            let mut content = i.content_mut();
            if content.title == "字符" {
                content.title = "普通文本".to_string();
            }
        }
    }
    let mut text_node: Option<Rc<TreeNode<TreeItem>>> = None;
    for i in tree.children().iter() {
        if i.content().title == "字符" {
            if let Some(ref node) = text_node {
                node.content_mut().content.push_str(&i.content().content);
                node.content_mut().span.end = i.content().span.end;
                i.content_mut().title.clear();
            } else {
                text_node = Some(i.clone());
                i.content_mut().title = "普通文本".into();
            }
        } else {
            text_node = None;
        }
    }
    {
        let children_new: Vec<Rc<TreeNode<TreeItem>>> = tree
            .children()
            .iter()
            .filter(|i| !i.content().title.is_empty())
            .cloned()
            .collect();
        let mut children = tree.children_mut();
        children.clear();
        for i in children_new.into_iter() {
            children.push(i);
        }
    }
    for i in tree.children().iter() {
        merge_literal(i);
    }
}
