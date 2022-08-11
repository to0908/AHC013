use tools::*;

#[derive(Clone, Debug)]
pub struct Answer {
    pub n: usize,
    pub k: usize,
    pub score: i32,
    pub svg: String,
}


fn main() {

    let mut doc = "".to_string();
    let mut v = Vec::new();

    
    // TODO: 
    // - Kごととかで表示
    

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

        let (score, err, svg) = match output {
            Ok(output) => vis(&input, &output),
            Err(err) => (0, err, String::new()),
        };

        if err.len() > 0 {
            println!("{}", err);
        }
        v.push(Answer {n : input.n.clone(), k : input.k.clone(), score : score.clone(), svg : svg.clone()});

        let vis = format!("<html><body>{}</body></html>", svg);
        std::fs::write(format!("visualize/{}.html", basename), &vis).unwrap();

        doc = format!("{}{}", doc, format!("<a href=\"./visualize/{}.html\">{}</a>\n", basename, basename))
    }

    let mut index = format!("<html><body>{}<br><br>", doc);

    let sz = v.len() / 10;
    for i in 0..sz {
        let mut clus = "".to_string();
        let l = i * 10;
        let r = std::cmp::min(v.len(), i*10+10);
        for j in l..r {
            clus = format!("{}seed={}, N={}, K={}, Score={}<br>{}<br><br>", clus, j, v[j].n, v[j].k, v[j].score, v[j].svg);
        }
        let vis = format!("<html><body>{}</body></html>", clus);
        std::fs::write(format!("visualize/clus_{}.html", i), &vis).unwrap();
        index = format!("{}{}", index, format!("<a href=\"./visualize/clus_{}.html\">{}-{}</a>\n", i, l, r));
    }

    index = format!("{}{}", index, "</body></html>");
    std::fs::write("index.html", &index).unwrap();
}
