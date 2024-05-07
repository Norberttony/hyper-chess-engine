
// Courtesy of
// https://www.chessprogramming.org/Match_Statistics#SPRT

function log_likelihood(x){
    return 1 / (1 + 10 ** (-x / 400))
}

function log_likelihood_ratio(wins, draws, losses, h_0, h_1){
    if (wins == 0 || draws == 0 || losses == 0)
        return 0;

    const total = wins + draws + losses;
    const w = wins / total;
    const d = draws / total;
    const l = losses / total;

    const score = w + d / 2;
    const m2 = w + d / 4;
    const v = m2 - score ** 2;
    const v_s = v / total;

    const s0 = log_likelihood(h_0);
    const s1 = log_likelihood(h_1);
    return (s1 - s0) * (2 * score - s0 - s1) / v_s / 2;
}

function SPRT(wins, draws, losses, h_0, h_1, alpha, beta){
    const ratio = log_likelihood_ratio(wins, draws, losses, h_0, h_1);
    const l_a = Math.log(beta / (1 - alpha));
    const l_b = Math.log((1 - beta) / alpha);

    console.log(ratio);
    console.log(l_a, l_b);

    if (ratio > l_b)
        return "H1";
    else if (ratio < l_a)
        return "H0";
    else
        return undefined;
}

module.exports = { SPRT };
