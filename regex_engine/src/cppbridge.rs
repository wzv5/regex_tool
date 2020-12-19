#[cxx::bridge]
mod ffi {
    struct TreeNode {
        title: String,
        content: String,
        start: u32,
        end: u32,
        children: Vec<TreeNode>,
    }

    struct MatchGroup {
        text: String,
        start: u32,
        end: u32,
    }

    struct Match {
        groups: Vec<MatchGroup>,
    }

    struct Matches {
        group_names: Vec<String>,
        matches: Vec<Match>,
    }

    extern "Rust" {
        type Regex;

        fn regex_parse(s: &str, ignore_whitespace: bool) -> Result<TreeNode>;
        fn regex_new(
            re: &str,
            ignore_whitespace: bool,
            case_insensitive: bool,
            multi_line: bool,
            dot_matches_new_line: bool,
        ) -> Result<Box<Regex>>;
        fn regex_match(re: &Box<Regex>, text: &str) -> Result<Matches>;
        fn regex_replace(re: &Box<Regex>, text: &str, rep: &str) -> String;
        fn regex_split(re: &Box<Regex>, text: &str) -> Vec<String>;
    }
}

fn conv_tree(tree: &super::tree::Tree<super::parse::TreeItem>) -> ffi::TreeNode {
    let mut children = vec![];
    for i in tree.children().iter() {
        children.push(conv_tree(i));
    }
    let content = tree.content();
    ffi::TreeNode {
        title: content.title.clone(),
        content: content.content.clone(),
        start: content.span.start,
        end: content.span.end,
        children,
    }
}

pub fn regex_parse(s: &str, ignore_whitespace: bool) -> anyhow::Result<ffi::TreeNode> {
    let ast = super::parse::parse(s, ignore_whitespace)?;
    Ok(conv_tree(&ast))
}

pub struct Regex {
    re: regex::Regex,
}

pub fn regex_new(
    re: &str,
    ignore_whitespace: bool,    // 忽略空白
    case_insensitive: bool,     // 忽略大小写
    multi_line: bool,           // 多行模式，使 ^ 和 $ 匹配任意一行的行首行尾
    dot_matches_new_line: bool, // 单行模式，点（.）可以匹配换行符
) -> anyhow::Result<Box<Regex>> {
    let re = regex::RegexBuilder::new(re)
        .ignore_whitespace(ignore_whitespace)
        .case_insensitive(case_insensitive)
        .multi_line(multi_line)
        .dot_matches_new_line(dot_matches_new_line)
        .build()?;
    Ok(Box::new(Regex { re }))
}

pub fn regex_match(re: &Box<Regex>, text: &str) -> anyhow::Result<ffi::Matches> {
    let re = &re.re;
    let group_names = re
        .capture_names()
        .into_iter()
        .map(|i| i.unwrap_or_default().to_string())
        .collect::<Vec<_>>();
    let mut matches = vec![];
    for i in re.captures_iter(text) {
        let mut amatch = vec![];
        for g in i.iter() {
            let group = match g {
                Some(g) => ffi::MatchGroup {
                    text: g.as_str().to_string(),
                    start: g.start() as _,
                    end: g.end() as _,
                },
                None => ffi::MatchGroup {
                    text: String::new(),
                    start: 0,
                    end: 0,
                },
            };
            amatch.push(group);
        }
        matches.push(ffi::Match { groups: amatch });
    }
    Ok(ffi::Matches {
        group_names,
        matches,
    })
}

pub fn regex_replace(re: &Box<Regex>, text: &str, rep: &str) -> String {
    let re = &re.re;
    re.replace_all(text, rep).to_string()
}

pub fn regex_split(re: &Box<Regex>, text: &str) -> Vec<String> {
    let re = &re.re;
    re.split(text).map(|i| i.to_string()).collect()
}
