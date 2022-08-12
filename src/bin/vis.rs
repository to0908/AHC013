use tools::*;

#[derive(Clone, Debug)]
pub struct Answer {
    pub n: usize,
    pub k: usize,
    pub seed: i32, 
    pub score: i32,
    pub action_count: usize,
    pub svg: String
}


fn main() {

    let mut doc = "".to_string();
    let mut v = Vec::new();
    let mut dense_v = Vec::new();

    
    // TODO: 
    // - Kごととかで表示
    

    for i in 0..100{
        let in_file = format!("in/{:>04}.txt", i);
        let out_file = format!("out/{:>04}.txt", i);
        let cerr_file = format!("cerr/{:>04}.txt", i);
        let basename = format!("{:>04}", i);
        let input = std::fs::read_to_string(&in_file).unwrap_or_else(|_| {
            eprintln!("no such file: {}", in_file);
            std::process::exit(1);
        });
        let output = std::fs::read_to_string(&out_file).unwrap_or_else(|_| {
            eprintln!("no such file: {}", out_file);
            std::process::exit(1);
        });
        let cerr = std::fs::read_to_string(&cerr_file).unwrap_or_else(|_| {
            eprintln!("no such file: {}", cerr_file);
            std::process::exit(1);
        });
        let solver_info = cerr.lines().nth(0).unwrap();

        let input = parse_input(&input);
        let output = parse_output(&input, &output);

        let (score, err, svg, action_count) = match output {
            Ok(output) => vis(&input, &output),
            Err(err) => (0, err, String::new(), 0),
        };

        if err.len() > 0 {
            println!("{}", err);
        }

        let action_count = action_count;

        v.push(Answer {
            n : input.n.clone(), 
            k : input.k.clone(), 
            seed: i.clone(),
            score : score.clone(), 
            action_count : action_count.clone(),
            svg : svg.clone()
        });

        if solver_info == "Solver: Dense" {
            dense_v.push(Answer {
                n : input.n.clone(), 
                k : input.k.clone(), 
                seed: i.clone(),
                score : score.clone(), 
                action_count : action_count.clone(),
                svg : svg.clone()
            });
        }

        let vis = format!("<html><body>{}</body></html>", svg);
        std::fs::write(format!("visualize/{}.html", basename), &vis).unwrap();

        doc = format!("{}{}", doc, format!("<a href=\"./visualize/{}.html\">{}</a>\n", basename, basename))
    }

    let mut index = format!("<html><body>{}<br><br>", doc);

    // 10個ずつ表示
    let sz = v.len() / 10;
    for i in 0..sz {
        let mut clus = "".to_string();
        let l = i * 10;
        let r = std::cmp::min(v.len(), i*10+10);
        for j in l..r {
            let max_score = (v[j].k * 50 * 99) as f32;
            let f_score = v[j].score as f32;
            let ratio = f_score / max_score;
            clus = format!("{}seed={}, N={}, K={}, Score={} (ratio: {}), Action={} (残り: {})<br>{}<br><br>", 
                clus, v[j].seed, v[j].n, v[j].k, v[j].score, ratio, v[j].action_count, v[j].k*100 - v[j].action_count, v[j].svg);
        }
        let vis = format!("<html><body>{}</body></html>", clus);
        std::fs::write(format!("visualize/seed_{}-{}.html", l, r), &vis).unwrap();
        index = format!("{}{}", index, format!("<a href=\"./visualize/seed_{}-{}.html\">{0}-{1}</a>\n", l, r));
    }

    index = format!("{}<br><br>Dense Solver<br>", index);
    let sz2 = dense_v.len() / 10;
    for i in 0..sz2 {
        let mut clus = "".to_string();
        let l = i * 10;
        let r = std::cmp::min(dense_v.len(), i*10+10);
        for j in l..r {
            let max_score = (dense_v[j].k * 50 * 99) as f32;
            let f_score = dense_v[j].score as f32;
            let ratio = f_score / max_score;
            clus = format!("{}seed={}, N={}, K={}, Score={} (ratio: {}), Action={} (残り: {})<br>{}<br><br>", 
                clus, dense_v[j].seed, dense_v[j].n, dense_v[j].k, dense_v[j].score, ratio, 
                dense_v[j].action_count, dense_v[j].k*100 - dense_v[j].action_count, dense_v[j].svg);
        }
        let vis = format!("<html><body>{}</body></html>", clus);
        std::fs::write(format!("visualize/dense_{}-{}.html", l, r), &vis).unwrap();
        index = format!("{}{}", index, format!("<a href=\"./visualize/dense_{}-{}.html\">{0}-{1}</a>\n", l, r));
    }

    // htmlを閉じる (ここから後ろは編集しない)
    index = format!("{}{}", index, "</body></html>");
    std::fs::write("index.html", &index).unwrap();
}
