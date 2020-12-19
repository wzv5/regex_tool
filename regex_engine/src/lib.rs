#![allow(unused_variables)]

mod cppbridge;
mod parse;
mod tree;

#[cfg(test)]
mod tests {
    const REGEX_PATTERN: &'static str =
        r"^(?P<tag1>123)abc|bbbc[a-z.123][^a\s[\d]]\d+.*\s\S(?imx:aaa)(?ui)123|zzz|[ -  ]$";

    #[test]
    fn main1() -> Result<(), Box<dyn std::error::Error>> {
        let tree = super::parse::parse(REGEX_PATTERN, false)?;
        print_tree(&tree, 0);
        Ok(())
    }

    fn print_tree(tree: &super::tree::Tree<super::parse::TreeItem>, level: usize) {
        println!(
            "{}{} - {} ({},{})",
            "\t".repeat(level),
            tree.content().title,
            tree.content().content,
            tree.content().span.start,
            tree.content().span.end,
        );
        let level = level + 1;
        for i in tree.children().iter() {
            print_tree(i, level);
        }
    }
}
