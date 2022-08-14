use tools::*;

#[derive(Clone, Debug)]
pub struct Answer {
    pub n: usize,
    pub k: usize,
    pub seed: i32, 
    pub score: i32,
    pub move_count: usize,
    pub connect_count: usize,
    pub svg: String
}


fn main() {

    let mut v = Vec::new();
    let mut dense_v = Vec::new();
    let mut middle_v = Vec::new();
    let mut sparse_v = Vec::new();
    let mut dense_score = 0 as f32;
    let mut middle_score = 0 as f32;
    let mut sparse_score = 0 as f32;

    
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

        let (score, err, svg, move_count, connect_count) = match output {
            Ok(output) => vis(&input, &output),
            Err(err) => (0, err, String::new(), 0, 0),
        };

        if err.len() > 0 {
            println!("{}", err);
        }


        v.push(Answer {
            n : input.n.clone(), 
            k : input.k.clone(), 
            seed: i.clone(),
            score : score.clone(), 
            move_count : move_count.clone(),
            connect_count : connect_count.clone(),
            svg : svg.clone()
        });

        if solver_info == "Solver: Dense" {
            dense_score += score as f32;
            dense_v.push(Answer {
                n : input.n.clone(), 
                k : input.k.clone(), 
                seed: i.clone(),
                score : score.clone(), 
                move_count : move_count.clone(),
                connect_count : connect_count.clone(),
                svg : svg.clone()
            });
        }
        if solver_info == "Solver: Middle" {
            middle_score += score as f32;
            middle_v.push(Answer {
                n : input.n.clone(), 
                k : input.k.clone(), 
                seed: i.clone(),
                score : score.clone(), 
                move_count : move_count.clone(),
                connect_count : connect_count.clone(),
                svg : svg.clone()
            });
        }
        if solver_info == "Solver: Sparse" {
            sparse_score += score as f32;
            sparse_v.push(Answer {
                n : input.n.clone(), 
                k : input.k.clone(), 
                seed: i.clone(),
                score : score.clone(), 
                move_count : move_count.clone(),
                connect_count : connect_count.clone(),
                svg : svg.clone()
            });
        }

        let vis = format!("<html><body>{}</body></html>", svg);
        std::fs::write(format!("visualize/{}.html", basename), &vis).unwrap();
    }

    let mut index = "<html><body>Seed<br>".to_string();

    // 10個ずつ表示
    let sz = (v.len()+9) / 10;
    for i in 0..sz {
        let mut clus = "".to_string();
        let l = i * 10;
        let r = std::cmp::min(v.len(), i*10+10);
        for j in l..r {
            let max_score = (v[j].k * 50 * 99) as f32;
            let f_score = v[j].score as f32;
            let ratio = f_score / max_score;
            let action_count = v[j].move_count + v[j].connect_count;
            let density = (v[j].k * 100) as f32 / (v[j].n * v[j].n) as f32;
            clus = format!("{}seed={}, N={}, K={}, Density={}, Score={} (ratio: {}), Action={} (残り: {}, Move={}, Connect={})<br>{}<br><br>", 
                clus, v[j].seed, v[j].n, v[j].k, density, v[j].score, ratio, 
                action_count, v[j].k*100 - action_count, v[j].move_count, v[j].connect_count, v[j].svg);
        }
        let vis = format!("<html><body>{}</body></html>", clus);
        std::fs::write(format!("visualize/seed_{}-{}.html", l, r), &vis).unwrap();
        index = format!("{}{}", index, format!("<a href=\"./visualize/seed_{}-{}.html\">{0}-{1}</a>\n", l, r));
    }

    // これ関数化したい気持ちがあるぜ！
    index = format!("{}<br><br>Dense Solver, Mean Score={}<br>", index, dense_score / dense_v.len() as f32);
    let sz = (dense_v.len()+9) / 10;
    for i in 0..sz {
        let mut clus = "".to_string();
        let l = i * 10;
        let r = std::cmp::min(dense_v.len(), i*10+10);
        for j in l..r {
            let max_score = (dense_v[j].k * 50 * 99) as f32;
            let f_score = dense_v[j].score as f32;
            let ratio = f_score / max_score;
            let action_count = dense_v[j].move_count + dense_v[j].connect_count;
            let density = (dense_v[j].k * 100) as f32 / (dense_v[j].n * dense_v[j].n) as f32;
            clus = format!("{}seed={}, N={}, K={}, Density={}, Score={} (ratio: {}), Action={} (残り: {}, Move={}, Connect={})<br>{}<br><br>", 
                clus, dense_v[j].seed, dense_v[j].n, dense_v[j].k, density, dense_v[j].score, ratio, 
                action_count, dense_v[j].k*100 - action_count, dense_v[j].move_count, dense_v[j].connect_count, dense_v[j].svg);
        }
        let vis = format!("<html><body>{}</body></html>", clus);
        std::fs::write(format!("visualize/dense_{}-{}.html", l, r), &vis).unwrap();
        index = format!("{}{}", index, format!("<a href=\"./visualize/dense_{}-{}.html\">{0}-{1}</a>\n", l, r));
    }

    let mut mean_score = 0 as f32;
    if middle_v.len() != 0 {
        mean_score = middle_score / middle_v.len() as f32;
    }
    index = format!("{}<br><br>Middle Solver, Mean Score={}<br>", index, mean_score);
    let sz = (middle_v.len()+9) / 10;
    for i in 0..sz {
        let mut clus = "".to_string();
        let l = i * 10;
        let r = std::cmp::min(middle_v.len(), i*10+10);
        for j in l..r {
            let max_score = (middle_v[j].k * 50 * 99) as f32;
            let f_score = middle_v[j].score as f32;
            let ratio = f_score / max_score;
            let action_count = middle_v[j].move_count + middle_v[j].connect_count;
            let density = (middle_v[j].k * 100) as f32 / (middle_v[j].n * middle_v[j].n) as f32;
            clus = format!("{}seed={}, N={}, K={}, Density={}, Score={} (ratio: {}), Action={} (残り: {}, Move={}, Connect={})<br>{}<br><br>", 
                clus, middle_v[j].seed, middle_v[j].n, middle_v[j].k, density, middle_v[j].score, ratio, 
                action_count, middle_v[j].k*100 - action_count, middle_v[j].move_count, middle_v[j].connect_count, middle_v[j].svg);
        }
        let vis = format!("<html><body>{}</body></html>", clus);
        std::fs::write(format!("visualize/middle_{}-{}.html", l, r), &vis).unwrap();
        index = format!("{}{}", index, format!("<a href=\"./visualize/middle_{}-{}.html\">{0}-{1}</a>\n", l, r));
    }

    index = format!("{}<br><br>Sparse Solver, Mean Score={}<br>", index, sparse_score / sparse_v.len() as f32);
    let sz = (sparse_v.len()+9) / 10;
    for i in 0..sz {
        let mut clus = "".to_string();
        let l = i * 10;
        let r = std::cmp::min(sparse_v.len(), i*10+10);
        for j in l..r {
            let max_score = (sparse_v[j].k * 50 * 99) as f32;
            let f_score = sparse_v[j].score as f32;
            let ratio = f_score / max_score;
            let action_count = sparse_v[j].move_count + sparse_v[j].connect_count;
            let density = (sparse_v[j].k * 100) as f32 / (sparse_v[j].n * sparse_v[j].n) as f32;
            clus = format!("{}seed={}, N={}, K={}, Density={}, Score={} (ratio: {}), Action={} (残り: {}, Move={}, Connect={})<br>{}<br><br>", 
                clus, sparse_v[j].seed, sparse_v[j].n, sparse_v[j].k, density, sparse_v[j].score, ratio, 
                action_count, sparse_v[j].k*100 - action_count, sparse_v[j].move_count, sparse_v[j].connect_count, sparse_v[j].svg);
        }
        let vis = format!("<html><body>{}</body></html>", clus);
        std::fs::write(format!("visualize/sparse_{}-{}.html", l, r), &vis).unwrap();
        index = format!("{}{}", index, format!("<a href=\"./visualize/sparse_{}-{}.html\">{0}-{1}</a>\n", l, r));
    }

    // htmlを閉じる (ここから後ろは編集しない)
    index = format!("{}{}", index, "</body></html>");
    std::fs::write("index.html", &index).unwrap();
}
