use tools::*;

fn main() {

    for i in 0..100{
        let in_file = format!("in/{:>04}.txt", i);
        let out_file = format!("out/{:>04}.txt", i);
        let basename = format!("{:>04}", i);
        let input = std::fs::read_to_string(&in_file).unwrap_or_else(|_| {
            eprintln!("no such file: {}", in_file);
            std::process::exit(1);
        });
        let output = std::fs::read_to_string(&out_file).unwrap_or_else(|_| {
            eprintln!("no such file: {}", out_file);
            std::process::exit(1);
        });

        let input = parse_input(&input);
        let output = parse_output(&input, &output);

        let (_, err, svg) = match output {
            Ok(output) => vis(&input, &output),
            Err(err) => (0, err, String::new()),
        };

        if err.len() > 0 {
            println!("{}", err);
        }
        let vis = format!("<html><body>{}</body></html>", svg);
        std::fs::write(format!("visualize/{}.html", basename), &vis).unwrap();
    }
}
