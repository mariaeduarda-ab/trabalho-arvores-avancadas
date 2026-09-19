// Gera o relatório técnico (.docx) do Trabalho Prático I.
// Executar de dentro de relatorio/:  node build_report.js
const fs = require("fs");
const path = require("path");
const {
  Document, Packer, Paragraph, TextRun, HeadingLevel, AlignmentType,
  Table, TableRow, TableCell, WidthType, BorderStyle, ImageRun,
  PageBreak, LevelFormat, ShadingType, TabStopType, TabStopPosition, LeaderType
} = require("docx");

const FIG = path.join(__dirname, "..", "figs");

// ---- lê largura/altura de um PNG (chunk IHDR) para preservar proporção ----
function pngSize(file) {
  const b = fs.readFileSync(file);
  return { w: b.readUInt32BE(16), h: b.readUInt32BE(20) };
}
// imagem escalada para uma largura alvo (px), mantendo a proporção
function img(file, targetW) {
  const p = path.join(FIG, file);
  const { w, h } = pngSize(p);
  const scale = targetW / w;
  return new ImageRun({
    type: "png",
    data: fs.readFileSync(p),
    transformation: { width: Math.round(targetW), height: Math.round(h * scale) },
  });
}

// ---------- helpers de texto ----------
const P = (children, opts = {}) =>
  new Paragraph({
    alignment: opts.align || AlignmentType.JUSTIFIED,
    spacing: { line: 276, after: opts.after ?? 120 },
    ...opts,
    children: Array.isArray(children) ? children : [new TextRun(children)],
  });

const T = (text, o = {}) => new TextRun({ text, ...o });
const B = (text) => new TextRun({ text, bold: true });

const H1 = (text) =>
  new Paragraph({ heading: HeadingLevel.HEADING_1, spacing: { before: 240, after: 120 },
    children: [new TextRun({ text, bold: true })] });
const H2 = (text) =>
  new Paragraph({ heading: HeadingLevel.HEADING_2, spacing: { before: 160, after: 80 },
    children: [new TextRun({ text, bold: true })] });

const bullet = (runs) =>
  new Paragraph({ numbering: { reference: "bul", level: 0 },
    alignment: AlignmentType.JUSTIFIED, spacing: { line: 276, after: 60 },
    children: Array.isArray(runs) ? runs : [new TextRun(runs)] });
const numItem = (runs) =>
  new Paragraph({ numbering: { reference: "num", level: 0 },
    alignment: AlignmentType.JUSTIFIED, spacing: { line: 276, after: 60 },
    children: Array.isArray(runs) ? runs : [new TextRun(runs)] });

const NOBORDER = { style: BorderStyle.NONE, size: 0, color: "FFFFFF" };
const noBorders = { top: NOBORDER, bottom: NOBORDER, left: NOBORDER, right: NOBORDER,
  insideHorizontal: NOBORDER, insideVertical: NOBORDER };

// galeria de imagens em N colunas, sem bordas, com legenda
function galeria(files, totalW, caption, cols = 3) {
  const colW = Math.floor(totalW / files.length);
  const imgW = Math.floor(colW / 15) - 8;   // dxa -> px (1440 dxa/in, 96 px/in)
  const cells = files.map((f) =>
    new TableCell({
      width: { size: colW, type: WidthType.DXA }, borders: noBorders,
      children: [new Paragraph({ alignment: AlignmentType.CENTER,
        children: [img(f, imgW)] })],
    }));
  const tbl = new Table({
    width: { size: totalW, type: WidthType.DXA },
    columnWidths: files.map(() => colW), borders: noBorders,
    rows: [new TableRow({ children: cells })],
  });
  const cap = new Paragraph({ alignment: AlignmentType.CENTER,
    spacing: { after: 160, before: 40 },
    children: [new TextRun({ text: caption, italics: true, size: 18, color: "555555" })] });
  return [tbl, cap];
}

// ---------- tabela de complexidade ----------
const CONTENT_W = 9026; // A4 - margens de 1"
function celTexto(text, o = {}) {
  return new TableCell({
    width: { size: o.w, type: WidthType.DXA },
    shading: o.shade ? { type: ShadingType.CLEAR, fill: o.shade, color: "auto" } : undefined,
    margins: { top: 40, bottom: 40, left: 80, right: 80 },
    children: [new Paragraph({ alignment: o.center ? AlignmentType.CENTER : AlignmentType.LEFT,
      spacing: { line: 240, after: 0 },
      children: [new TextRun({ text, bold: !!o.bold, size: 18 })] })],
  });
}
function tabelaComplexidade() {
  const cols = [1650, 1694, 1694, 1694, 1150, 1144]; // soma = 9026
  const head = ["Estrutura", "Busca", "Inserção", "Remoção", "Espaço", "Constr."];
  const rows = [
    ["Trie", "O(m)", "O(m)", "O(m)", "O(N·m·σ)*", "O(N·m)"],
    ["Patricia", "O(m)", "O(m)", "O(m)", "O(N)", "O(N·m)"],
    ["Splay", "O(1) / O(log n)ᵃ / O(n)", "O(log n)ᵃ", "O(log n)ᵃ", "O(n)", "O(n log n)"],
    ["Treap", "O(log n)ᵉ / O(n)", "O(log n)ᵉ", "O(log n)ᵉ", "O(n)", "O(n log n)"],
    ["KD-Tree", "O(log n) / O(n)", "O(log n) / O(n)", "O(log n) / O(n)", "O(n)", "O(n log n)"],
    ["BST (ref.)", "O(log n) / O(n)", "O(log n) / O(n)", "O(log n) / O(n)", "O(n)", "O(n log n)"],
    ["AVL (ref.)", "O(log n)", "O(log n)", "O(log n)", "O(n)", "O(n log n)"],
  ];
  const headRow2 = new TableRow({ tableHeader: true, children: head.map((h, i) =>
    new TableCell({ width: { size: cols[i], type: WidthType.DXA },
      shading: { type: ShadingType.CLEAR, fill: "1F4E79", color: "auto" },
      margins: { top: 40, bottom: 40, left: 80, right: 80 },
      children: [new Paragraph({ alignment: AlignmentType.CENTER, spacing: { after: 0 },
        children: [new TextRun({ text: h, bold: true, color: "FFFFFF", size: 18 })] })] })) });
  const bodyRows = rows.map((r, ri) => new TableRow({ children: r.map((c, i) =>
    celTexto(c, { w: cols[i], center: i > 0, bold: i === 0, shade: ri % 2 ? "EAF0F7" : "FFFFFF" })) }));
  return new Table({ width: { size: CONTENT_W, type: WidthType.DXA }, columnWidths: cols,
    rows: [headRow2, ...bodyRows] });
}

// ============================ DOCUMENTO ============================
const children = [];

// --- capa ---
children.push(
  new Paragraph({ spacing: { before: 1400, after: 0 }, alignment: AlignmentType.CENTER,
    children: [T("Trabalho Prático Individual I", { size: 20, color: "666666" })] }),
  new Paragraph({ alignment: AlignmentType.CENTER, spacing: { before: 200, after: 0 },
    children: [T("Estruturas em Árvores Avançadas", { bold: true, size: 40 })] }),
  new Paragraph({ alignment: AlignmentType.CENTER, spacing: { before: 120, after: 0 },
    children: [T("Modelagem, Implementação e Análise Comparativa de", { size: 24, italics: true }) ] }),
  new Paragraph({ alignment: AlignmentType.CENTER, spacing: { before: 0, after: 600 },
    children: [T("Estruturas em Árvore Especializadas", { size: 24, italics: true }) ] }),
  new Paragraph({ alignment: AlignmentType.CENTER, spacing: { before: 1200, after: 60 },
    children: [T("Aluno(a): ", { bold: true }), T("[SEU NOME COMPLETO]")] }),
  new Paragraph({ alignment: AlignmentType.CENTER, spacing: { after: 60 },
    children: [T("Matrícula: ", { bold: true }), T("[SUA MATRÍCULA]")] }),
  new Paragraph({ alignment: AlignmentType.CENTER, spacing: { after: 60 },
    children: [T("Disciplina: ", { bold: true }), T("Estrutura de Dados — 5º semestre")] }),
  new Paragraph({ alignment: AlignmentType.CENTER, spacing: { after: 60 },
    children: [T("Professor(a): ", { bold: true }), T("[NOME DO PROFESSOR]")] }),
  new Paragraph({ alignment: AlignmentType.CENTER, spacing: { after: 60 },
    children: [T("Data de entrega: ", { bold: true }), T("19 de setembro de 2026")] }),
  new Paragraph({ alignment: AlignmentType.CENTER, spacing: { before: 400, after: 0 },
    children: [T("Repositório com o código-fonte completo: ", { size: 18, color: "555555" }),
               T("[LINK DO GITHUB]", { size: 18, color: "1155CC" })] }),
  new Paragraph({ children: [new PageBreak()] }),
);

// --- sumário ---
const tocEntry = (text, page, bold = false) =>
  new Paragraph({ spacing: { after: 90, line: 276 },
    tabStops: [{ type: TabStopType.RIGHT, position: 9026, leader: LeaderType.DOT }],
    children: [
      new TextRun({ text, bold, size: 24 }),
      new TextRun({ text: "\t" + String(page), bold, size: 24 }),
    ] });
children.push(
  new Paragraph({ heading: HeadingLevel.HEADING_1, spacing: { after: 200 },
    children: [T("Sumário", { bold: true })] }),
  tocEntry("Resumo", 3),
  tocEntry("1. Introdução e Fundamentação Teórica", 3),
  tocEntry("2. Projeto e Implementação das Estruturas", 4),
  tocEntry("3. Demonstração e Rastreamento Visual", 5),
  tocEntry("4. Análise de Complexidade e Comparação Teórica", 7),
  tocEntry("5. Metodologia Experimental e Resultados", 8),
  tocEntry("6. Aplicações, Análise Crítica e Discussão", 10),
  tocEntry("7. Conclusão", 11),
  tocEntry("Referências", 11),
  new Paragraph({ children: [new PageBreak()] }),
);

// =================== RESUMO ===================
children.push(H1("Resumo"));
children.push(P([
  T("Este trabalho apresenta a modelagem, a implementação e a análise comparativa de cinco estruturas de dados em árvore especializadas: "),
  B("Trie"), T(", "), B("Árvore Patricia (Radix Tree compacta)"), T(", "), B("Árvore Splay"),
  T(", "), B("Árvore Treap"), T(" e "), B("KD-Tree"),
  T(". Todas foram implementadas em C++17 com as operações de inserção, busca e remoção, além de operações específicas de cada estrutura. Para cada uma são apresentados o rastreamento visual de seus estados, a análise de complexidade assintótica (casos melhor, médio e pior) e experimentos computacionais que confrontam o comportamento teórico com o observado na prática. Os resultados evidenciam que não existe uma estrutura universalmente superior: cada uma é vantajosa em um contexto específico — strings e prefixos (Trie/Patricia), acessos com localidade temporal (Splay), balanceamento simples e probabilístico (Treap) e busca espacial multidimensional (KD-Tree)."),
]));
children.push(P([B("Palavras-chave: "),
  T("estruturas de dados; árvores de busca; Trie; Patricia; Splay; Treap; KD-Tree; análise de complexidade.")]));

// =================== SEÇÃO 1 ===================
children.push(H1("1. Introdução e Fundamentação Teórica"));
children.push(P([
  T("As "), B("estruturas de dados hierárquicas"), T(" (árvores) são fundamentais na organização e na recuperação eficiente de informações. Diferentemente de estruturas lineares, uma árvore permite, em geral, reduzir o custo de busca de "),
  T("O(n)"), T(" para "), T("O(log n)"),
  T(", desde que sua altura seja mantida próxima do logaritmo do número de elementos. As Árvores Binárias de Busca (BST) exploram essa ideia, mas, quando os dados são inseridos em ordem desfavorável, degeneram para uma lista encadeada, com altura O(n). As Árvores AVL resolvem esse problema garantindo balanceamento por meio de rotações e do controle do fator de balanceamento, ao custo de manter informação extra em cada nó."),
]));
children.push(P([
  B("Motivação. "),
  T("Apesar de eficientes, BST e AVL assumem que as chaves são simples valores comparáveis e que todos os acessos são igualmente prováveis. Muitos problemas reais fogem dessas hipóteses: buscas por "),
  B("prefixos de strings"), T(", padrões de acesso com "), B("localidade temporal"),
  T(" (poucas chaves acessadas com muita frequência), necessidade de uma implementação "),
  B("simples e balanceada em média"), T(", ou consultas em "), B("espaços multidimensionais"),
  T(". As cinco estruturas estudadas surgem exatamente para atender a essas necessidades, cada uma adotando uma estratégia de organização diferente."),
]));

children.push(H2("1.1. Conceitos fundamentais de cada estrutura"));
children.push(P([B("Trie (árvore de prefixos). "),
  T("Árvore n-ária em que cada aresta representa um caractere; uma chave é o caminho da raiz até um nó marcado como fim de palavra. Prefixos comuns compartilham o mesmo caminho. Invariante: o caminho da raiz a um nó descreve exatamente o prefixo acumulado. Anatomia do nó: um vetor/mapa de filhos (indexado por caractere) e um marcador booleano de fim de palavra. Resolve problemas de busca por prefixos e conjuntos de strings.")]));
children.push(P([B("Árvore Patricia (Radix Tree compacta). "),
  T("É uma Trie comprimida: cadeias de nós com um único filho são colapsadas, de modo que cada aresta guarda uma "),
  B("substring"), T(" em vez de um único caractere. Invariante: nenhum nó interno (exceto a raiz) possui um único filho sem também ser fim de chave. A anatomia do nó acrescenta o rótulo da aresta (substring). As operações usam "),
  B("divisão (split)"), T(" de arestas na inserção e "), B("fusão (merge)"),
  T(" na remoção. Reduz drasticamente o número de nós em relação à Trie.")]));
children.push(P([B("Árvore Splay. "),
  T("BST autoajustável: a cada acesso, o nó tocado é levado à raiz por rotações (operação splay, nos casos zig, zig-zig e zig-zag). Não armazena altura nem fator de balanceamento; o equilíbrio é apenas "),
  B("amortizado"), T(". Assim, elementos acessados recentemente ficam próximos da raiz, favorecendo padrões com localidade.")]));
children.push(P([B("Árvore Treap. "),
  T("Combina duas propriedades: é uma BST pelas "), B("chaves"),
  T(" e um max-heap pelas "), B("prioridades"),
  T(", sorteadas aleatoriamente na inserção. Como as prioridades são aleatórias, a forma da árvore equivale à de uma BST construída em ordem aleatória, resultando em altura esperada O(log n) sem regras de balanceamento complexas. O equilíbrio entre as duas propriedades é mantido por rotações simples.")]));
children.push(P([B("KD-Tree (k-dimensional, aqui k = 2). "),
  T("Organiza pontos no espaço, alternando a dimensão de corte a cada nível (nível par corta em x; nível ímpar corta em y). O plano é sucessivamente particionado em retângulos, o que permite consultas espaciais eficientes, como a busca do vizinho mais próximo, aproveitando a poda de regiões que não podem conter a resposta.")]));

children.push(H2("1.2. Diferenças conceituais em relação a BST e AVL"));
children.push(bullet([B("Tipo de chave: "), T("BST/AVL/Splay/Treap comparam chaves escalares; Trie/Patricia operam sobre a estrutura interna das strings; KD-Tree compara coordenadas por dimensão.")]));
children.push(bullet([B("Estratégia de balanceamento: "), T("AVL usa balanceamento estrito por altura; Splay usa reorganização por acesso (amortizado); Treap usa aleatoriedade (probabilístico); Trie/Patricia não precisam balancear, pois a altura depende do comprimento das chaves, não de n.")]));
children.push(bullet([B("Custo em função de n: "), T("em Trie/Patricia as operações são O(m) e independem de n; nas demais, dependem da altura da árvore.")]));

// =================== SEÇÃO 2 ===================
children.push(H1("2. Projeto e Implementação das Estruturas"));
children.push(P([
  T("As cinco estruturas foram implementadas em "), B("C++17"),
  T(", cada uma em um cabeçalho independente (header-only) na pasta "), new TextRun({ text: "include/", font: "Consolas" }),
  T(". As entradas pseudoaleatórias usam semente fixa (42), garantindo "), B("reprodutibilidade"),
  T(". A seguir descrevem-se as principais decisões de projeto; o código-fonte completo está no repositório indicado na capa.")]));

children.push(H2("2.1. Trie e Patricia"));
children.push(P([
  T("Na "), B("Trie"), T(", cada nó mantém um "), new TextRun({ text: "std::map<char, No*>", font: "Consolas" }),
  T(" (mapa ordenado, para saída alfabética) e um booleano "), new TextRun({ text: "fimDePalavra", font: "Consolas" }),
  T(". A inserção percorre a chave criando nós ausentes; a busca segue o caminho e verifica o marcador final; a remoção desmarca o fim de palavra e, ao retornar da recursão, apaga nós que ficaram sem filhos e sem marcação. A decisão-chave é armazenar o marcador de fim no próprio nó, permitindo que uma palavra seja prefixo de outra.")]));
children.push(P([
  T("A "), B("Patricia"), T(" acrescenta a cada nó o rótulo da aresta que vem do pai (a substring). A inserção calcula o maior prefixo comum entre a chave e o rótulo da aresta atual; quando o prefixo é apenas parcial, a aresta é "),
  B("dividida"), T(" criando um nó intermediário (split). A remoção, além de apagar, pode "),
  B("fundir"), T(" um nó que ficou com um único filho ao seu descendente (merge), concatenando os rótulos e mantendo a árvore compacta. Essa manutenção do invariante de compactação é a principal diferença de projeto em relação à Trie.")]));

children.push(H2("2.2. Splay e Treap"));
children.push(P([
  T("A "), B("Splay"), T(" usa ponteiros de pai para viabilizar o "),
  B("splay"), T(" ascendente. Inserção e busca são BST comuns seguidas de splay do nó acessado; a remoção dá splay na chave, remove a raiz e "),
  B("junta"), T(" as duas subárvores (leva o máximo da subárvore esquerda à raiz e anexa a direita). Os três casos de rotação (zig, zig-zig, zig-zag) estão implementados explicitamente.")]));
children.push(P([
  T("A "), B("Treap"), T(" é implementada de forma recursiva: na inserção, após descer para o filho, se a prioridade do filho for maior que a do pai, aplica-se uma rotação para fazê-lo subir, restaurando a propriedade de max-heap. Na remoção, o nó a excluir é rotacionado para baixo (sempre subindo o filho de maior prioridade) até virar folha. As prioridades são geradas por um "),
  new TextRun({ text: "std::mt19937", font: "Consolas" }), T(" com semente fixa.")]));

children.push(H2("2.3. KD-Tree"));
children.push(P([
  T("Cada nó guarda um ponto (x, y) e a dimensão de corte é determinada pela profundidade ("),
  new TextRun({ text: "prof % 2", font: "Consolas" }),
  T("). A inserção e a busca exata comparam apenas a coordenada da dimensão do nível. A remoção emprega a técnica clássica: ao remover um nó interno, ele é substituído pelo "),
  B("mínimo, na dimensão de corte, da subárvore direita"), T(" (função "),
  new TextRun({ text: "findMin", font: "Consolas" }),
  T("); se não houver subárvore direita, usa-se a esquerda, que passa a ser a direita. A busca do "),
  B("vizinho mais próximo"), T(" desce primeiro para o lado que contém o alvo e só visita o outro lado se a distância ao plano de corte for menor que a melhor distância já encontrada (poda).")]));
children.push(P([B("Trechos essenciais. "),
  T("Todas as estruturas expõem um método de exportação da topologia (formato textual simples) consumido por scripts Python que geram as figuras deste relatório, evitando reproduzir o código-fonte integral aqui.")]));

// =================== SEÇÃO 3 ===================
children.push(H1("3. Demonstração e Rastreamento Visual"));
children.push(P([
  T("Para cada estrutura são exibidos três estados: (1) inicial, após um conjunto de inserções; (2) intermediário, evidenciando uma operação relevante (bifurcação, split, rotação ou particionamento); e (3) após uma remoção/reorganização. Legenda das figuras de Trie/Patricia: nós verdes com "),
  new TextRun({ text: "*", font: "Consolas" }), T(" indicam fim de chave; nós cinza são internos; os rótulos em vermelho sobre as arestas são os caracteres/substrings.")]));

children.push(H2("3.1. Trie"));
children.push(P([T("Inserção de "), new TextRun({ text: "casa, caso", font: "Consolas" }),
  T(" (compartilham o prefixo "), new TextRun({ text: "cas", font: "Consolas" }),
  T("); em seguida "), new TextRun({ text: "carro", font: "Consolas" }),
  T(" cria uma bifurcação em "), new TextRun({ text: "ca", font: "Consolas" }),
  T("; por fim remove-se "), new TextRun({ text: "caso", font: "Consolas" }), T(".")]));
galeria(["trie_s1.png","trie_s2.png","trie_s3.png"], CONTENT_W,
  "Figura 1 — Trie: estado inicial, bifurcação após inserir \"carro\" e estado após remover \"caso\".").forEach(x=>children.push(x));

children.push(H2("3.2. Árvore Patricia"));
children.push(P([T("Inserção de "), new TextRun({ text: "corda, cordao, corte", font: "Consolas" }),
  T("; depois "), new TextRun({ text: "casa", font: "Consolas" }),
  T(" provoca um split do prefixo em "), new TextRun({ text: "c", font: "Consolas" }),
  T("; por fim remove-se "), new TextRun({ text: "corte", font: "Consolas" }),
  T(". Observe como as arestas guardam substrings inteiras, reduzindo o número de nós em relação à Trie.")]));
galeria(["patricia_s1.png","patricia_s2.png","patricia_s3.png"], CONTENT_W,
  "Figura 2 — Patricia: compactação por substrings, split ao inserir \"casa\" e recompactação após remoção.").forEach(x=>children.push(x));

children.push(H2("3.3. Árvore Splay"));
children.push(P([T("Após inserir "), new TextRun({ text: "50 30 70 20 40 60 80", font: "Consolas" }),
  T(", uma busca por "), new TextRun({ text: "20", font: "Consolas" }),
  T(" leva o nó acessado à raiz por rotações (estado intermediário); em seguida remove-se "),
  new TextRun({ text: "20", font: "Consolas" }), T(". A demonstração evidencia o autoajuste característico da Splay.")]));
galeria(["splay_s1.png","splay_s2.png","splay_s3.png"], CONTENT_W,
  "Figura 3 — Splay: estado inicial, splay de 20 até a raiz após a busca e estado após remover 20.").forEach(x=>children.push(x));

children.push(H2("3.4. Árvore Treap"));
children.push(P([T("Cada nó mostra "), new TextRun({ text: "chave(pPrioridade)", font: "Consolas" }),
  T("; verifica-se que as chaves respeitam a BST e as prioridades respeitam o max-heap (a prioridade do pai é sempre maior que a dos filhos). A inserção de "),
  new TextRun({ text: "35 e 80", font: "Consolas" }), T(" provoca rotações; depois remove-se "),
  new TextRun({ text: "30", font: "Consolas" }), T(".")]));
galeria(["treap_s1.png","treap_s2.png","treap_s3.png"], CONTENT_W,
  "Figura 4 — Treap: BST pelas chaves e max-heap pelas prioridades; rotações na inserção e remoção.").forEach(x=>children.push(x));

children.push(H2("3.5. KD-Tree"));
children.push(P([T("São inseridos seis pontos; o particionamento do plano mostra os cortes verticais (em x, nível par, em vermelho) e horizontais (em y, nível ímpar, em azul). A inserção de "),
  new TextRun({ text: "(60,50)", font: "Consolas" }),
  T(" subdivide uma região; a remoção de "), new TextRun({ text: "(30,40)", font: "Consolas" }),
  T(" (a raiz) aciona a substituição pelo mínimo (findMin). A busca do vizinho mais próximo de (55,55) retorna corretamente (60,50).")]));
galeria(["kdtree_s1_space.png","kdtree_s2_space.png","kdtree_s3_space.png"], CONTENT_W,
  "Figura 5 — KD-Tree: particionamento do plano nos três estados (inicial, após inserir (60,50) e após remover (30,40)).").forEach(x=>children.push(x));
galeria(["kdtree_s1.png"], Math.floor(CONTENT_W*0.6),
  "Figura 6 — Topologia da KD-Tree no estado inicial. Cada rótulo indica o ponto e a dimensão de corte [x] ou [y].", 1).forEach(x=>children.push(x));

// =================== SEÇÃO 4 ===================
children.push(H1("4. Análise de Complexidade e Comparação Teórica"));
children.push(P([T("A Tabela 1 resume a complexidade assintótica das operações. Notação: "),
  B("n"), T(" = número de elementos; "), B("m"), T(" = comprimento da chave (string); "),
  B("N"), T(" = número de chaves; "), B("σ"), T(" = tamanho do alfabeto. Quando há duas expressões separadas por barra, indicam "),
  B("caso médio / pior caso"), T("; "), new TextRun({ text: "ᵃ", }), T(" = amortizado; "),
  new TextRun({ text: "ᵉ" }), T(" = esperado (probabilístico); "),
  new TextRun({ text: "*" }), T(" = pior caso de espaço da Trie.")]));
children.push(tabelaComplexidade());
children.push(new Paragraph({ alignment: AlignmentType.CENTER, spacing: { before: 40, after: 160 },
  children: [T("Tabela 1 — Complexidade assintótica das operações (com BST e AVL como referência).",
    { italics: true, size: 18, color: "555555" })] }));

children.push(H2("4.1. Discussão dos custos"));
children.push(bullet([B("Trie e Patricia: "), T("as operações custam O(m) porque dependem apenas de percorrer os m caracteres da chave, e não de n. A Patricia executa menos passos por não visitar nós intermediários redundantes, e usa O(N) nós contra o pior caso O(N·m·σ) da Trie — daí sua vantagem de memória.")]));
children.push(bullet([B("Splay: "), T("uma operação isolada pode custar O(n) no pior caso (árvore encadeada), mas o Teorema do Balanceamento Amortizado garante O(log n) amortizado por operação em qualquer sequência. Acessos repetidos à raiz custam O(1).")]));
children.push(bullet([B("Treap: "), T("como as prioridades são aleatórias, a altura esperada é O(log n); o pior caso O(n) existe, mas tem probabilidade desprezível. O custo é probabilístico, não amortizado.")]));
children.push(bullet([B("KD-Tree: "), T("a construção balanceada custa O(n log n) e a busca é O(log n) em média; contudo, em altas dimensões ou com inserções desfavoráveis, a poda perde eficácia e a busca tende a O(n).")]));
children.push(bullet([B("Comparação com BST/AVL: "), T("a AVL garante O(log n) no pior caso para busca/inserção/remoção; a BST simples só o faz em média. Splay e Treap abrem mão da garantia estrita da AVL em troca de simplicidade (Treap) ou de adaptação ao padrão de acesso (Splay).")]));

// =================== SEÇÃO 5 ===================
children.push(H1("5. Metodologia Experimental e Resultados"));
children.push(P([
  B("Metodologia. "),
  T("Os experimentos foram implementados em "), new TextRun({ text: "src/bench.cpp", font: "Consolas" }),
  T(" e compilados com "), new TextRun({ text: "g++ -O2 -std=c++17", font: "Consolas" }),
  T(". Os tempos são medidos com "), new TextRun({ text: "std::chrono::high_resolution_clock", font: "Consolas" }),
  T(" e representam a soma de todas as operações de cada fase, em milissegundos. As entradas são pseudoaleatórias com semente fixa. Para evitar que o compilador eliminasse laços de busca cujo resultado não é usado, os resultados são acumulados em uma variável volátil. Foram avaliados diferentes tamanhos de entrada e um padrão de acesso com localidade controlada.")]));

children.push(H2("5.1. Trie × Patricia (strings)"));
galeria(["g_str.png"], Math.floor(CONTENT_W*0.62),
  "Figura 7 — Tempo total de inserção e busca de n strings aleatórias (comprimento 8).", 1).forEach(x=>children.push(x));
children.push(P([T("A Patricia é consistentemente mais rápida que a Trie tanto na inserção quanto na busca, e cresce mais suavemente com n. O resultado confirma a teoria: ambas são O(m), mas a compactação reduz o número de nós efetivamente visitados e o número de alocações, melhorando também o uso de memória e a localidade de cache.")]));

children.push(H2("5.2. Splay × Treap × BST balanceada (busca aleatória)"));
galeria(["g_int.png"], Math.floor(CONTENT_W*0.62),
  "Figura 8 — Tempo total de n buscas aleatórias de inteiros (Splay, Treap e std::set).", 1).forEach(x=>children.push(x));
children.push(P([T("Sob acesso puramente aleatório (sem localidade), a Splay é a mais lenta: cada busca realiza rotações (escritas em memória) mesmo quando não há ganho de localidade. A Treap e a "),
  new TextRun({ text: "std::set", font: "Consolas" }),
  T(" (uma BST balanceada, do tipo rubro-negra) apresentam desempenho próximo, coerente com o custo O(log n) de ambas. Esse é um caso em que o comportamento assintótico esperado se confirma na prática.")]));

children.push(H2("5.3. Efeito da localidade de acesso (vantagem da Splay)"));
galeria(["g_locality.png"], Math.floor(CONTENT_W*0.62),
  "Figura 9 — 500 mil consultas em uma estrutura com n = 50000, variando a concentração no hot set (100 chaves).", 1).forEach(x=>children.push(x));
children.push(P([T("Este experimento concentra progressivamente as consultas em um pequeno conjunto de 100 chaves. Todas as estruturas melhoram (efeito de cache), mas a Splay é a que mais se beneficia proporcionalmente: seu tempo cai de cerca de 200 ms (acesso uniforme) para cerca de 90 ms (99% concentrado), uma redução de ~55%. Isso ocorre porque as chaves quentes são levadas para perto da raiz e passam a ser encontradas em poucos passos. Ainda assim, em tempo absoluto ela permanece mais lenta que a BST balanceada, pois o custo constante das rotações não é compensado apenas pela menor profundidade. Trata-se de uma "),
  B("divergência instrutiva"),
  T(" entre a análise assintótica (que favorece a Splay sob localidade) e o desempenho real (dominado por fatores constantes).")]));

children.push(H2("5.4. KD-Tree × busca linear (vizinho mais próximo)"));
galeria(["g_kd.png"], Math.floor(CONTENT_W*0.62),
  "Figura 10 — Tempo de 2000 consultas de vizinho mais próximo: KD-Tree (com poda) × busca linear.", 1).forEach(x=>children.push(x));
children.push(P([T("A busca linear cresce linearmente com n (cada consulta examina todos os pontos), enquanto a KD-Tree permanece praticamente constante graças à poda de regiões. Para n = 19000, a KD-Tree é cerca de 30× mais rápida. Este é o caso de maior convergência entre teoria e prática: a vantagem O(log n) médio × O(n) aparece de forma nítida.")]));

// =================== SEÇÃO 6 ===================
children.push(H1("6. Aplicações, Análise Crítica e Discussão"));
children.push(bullet([B("Trie: "), T("autocompletar, dicionários, corretores ortográficos e busca por prefixos — situações em que a chave é uma string e o compartilhamento de prefixos é frequente.")]));
children.push(bullet([B("Patricia: "), T("indexação compacta, tabelas de roteamento IP e índices baseados em prefixos, onde a economia de memória da compactação é decisiva.")]));
children.push(bullet([B("Splay: "), T("caches, montadores e sistemas com acessos altamente desiguais ou localidade temporal, em que itens recentes tendem a ser reacessados.")]));
children.push(bullet([B("Treap: "), T("estruturas de busca de propósito geral que exigem balanceamento com implementação simples, e como base de estruturas mais avançadas (ex.: treaps implícitas para sequências).")]));
children.push(bullet([B("KD-Tree: "), T("busca espacial, sistemas GIS, computação gráfica, reconhecimento de padrões e algoritmos de aprendizado de máquina baseados em vizinhos (k-NN).")]));

children.push(H2("6.1. Vantagens, limitações e dificuldades de implementação"));
children.push(P([T("A "), B("Trie"), T(" é simples, mas pode consumir muita memória; a "),
  B("Patricia"), T(" corrige isso, ao custo de uma implementação mais delicada (split e merge foram as partes mais trabalhosas). A "),
  B("Splay"), T(" tem código de rotação relativamente simples, mas depende de ponteiros de pai e sua vantagem só aparece sob localidade. A "),
  B("Treap"), T(" foi a de implementação mais direta (recursão com rotações), com a ressalva de depender de um bom gerador de aleatoriedade. A "),
  B("KD-Tree"), T(" tem inserção/busca simples, mas a remoção (via findMin alternando dimensões) foi a operação mais sutil de todo o trabalho.")]));
children.push(P([B("Diferenças teórico × experimental. "),
  T("O caso mais marcante foi a Splay: teoricamente favorecida sob localidade, na prática permaneceu mais lenta em tempo absoluto por causa do custo constante das rotações — embora a tendência prevista (melhora com a localidade) tenha se confirmado. Nos demais casos, teoria e experimento convergiram bem, com destaque para a KD-Tree × busca linear.")]));
children.push(P([B("Possíveis melhorias. "),
  T("Usar vetores de 26 posições na Trie (em vez de mapa) para acelerar o acesso; construir a KD-Tree de forma balanceada pela mediana; implementar a Splay em versão top-down para reduzir constantes; e adicionar treaps implícitas para operações de sequência.")]));

// =================== SEÇÃO 7 ===================
children.push(H1("7. Conclusão"));
children.push(P([T("O trabalho respondeu, de forma fundamentada, às questões norteadoras:")]));
children.push(numItem([B("Principais diferenças: "), T("as cinco estruturas diferem no tipo de chave (string, escalar, ponto) e na estratégia de organização (compartilhamento de prefixos, autoajuste por acesso, aleatoriedade, particionamento espacial).")]));
children.push(numItem([B("Melhor desempenho por operação/dados: "), T("Patricia para strings; BST balanceada/Treap para busca aleatória de inteiros; Splay para acessos com forte localidade; KD-Tree para busca espacial/vizinho mais próximo.")]));
children.push(numItem([B("Coerência com a complexidade: "), T("em geral sim — sobretudo na KD-Tree × busca linear e na Trie × Patricia. A exceção instrutiva foi a Splay, cujos fatores constantes mascararam parte da vantagem assintótica.")]));
children.push(numItem([B("Maior dificuldade de implementação: "), T("a recompactação (merge) da Patricia e a remoção da KD-Tree, por exigirem cuidado com casos de borda.")]));
children.push(numItem([B("Cenários ideais: "), T("cada estrutura tem seu nicho, conforme discutido na Seção 6 — não há vencedora universal.")]));
children.push(numItem([B("Vantagens e limitações: "), T("resumidas na Seção 6.1; a escolha correta depende do padrão de dados e de acesso do problema.")]));
children.push(P([
  T("Em síntese, o estudo evidenciou a relação direta entre as "),
  B("propriedades estruturais"), T(", a "), B("complexidade algorítmica"), T(" e o "),
  B("desempenho observado"),
  T(". Compreender essas relações é o que permite justificar tecnicamente a escolha de uma estrutura para um problema computacional específico — objetivo central deste trabalho.")]));

// =================== REFERÊNCIAS ===================
children.push(H1("Referências"));
const refs = [
  "CORMEN, T. H.; LEISERSON, C. E.; RIVEST, R. L.; STEIN, C. Algoritmos: Teoria e Prática. 3. ed. Rio de Janeiro: Elsevier, 2012.",
  "SEDGEWICK, R.; WAYNE, K. Algorithms. 4. ed. Boston: Addison-Wesley, 2011.",
  "SLEATOR, D. D.; TARJAN, R. E. Self-Adjusting Binary Search Trees. Journal of the ACM, v. 32, n. 3, p. 652–686, 1985.",
  "ARAGON, C. R.; SEIDEL, R. Randomized Search Trees. In: Proc. 30th FOCS, 1989, p. 540–545.",
  "BENTLEY, J. L. Multidimensional Binary Search Trees Used for Associative Searching. Communications of the ACM, v. 18, n. 9, p. 509–517, 1975.",
  "MORRISON, D. R. PATRICIA — Practical Algorithm To Retrieve Information Coded in Alphanumeric. Journal of the ACM, v. 15, n. 4, p. 514–534, 1968.",
];
refs.forEach((r) => children.push(new Paragraph({ alignment: AlignmentType.JUSTIFIED,
  spacing: { after: 100, line: 276 }, indent: { left: 360, hanging: 360 },
  children: [new TextRun({ text: r, size: 20 })] })));

// ============================ MONTAGEM ============================
const doc = new Document({
  creator: "Trabalho Pratico I",
  title: "Estruturas em Arvores Avancadas",
  styles: {
    default: {
      document: { run: { font: "Times New Roman", size: 24 } }, // 12pt
    },
    paragraphStyles: [
      { id: "Heading1", name: "Heading 1", basedOn: "Normal", next: "Normal", quickFormat: true,
        run: { font: "Times New Roman", size: 28, bold: true, color: "1F4E79" },
        paragraph: { spacing: { before: 240, after: 120 }, outlineLevel: 0 } },
      { id: "Heading2", name: "Heading 2", basedOn: "Normal", next: "Normal", quickFormat: true,
        run: { font: "Times New Roman", size: 25, bold: true, color: "2E5C8A" },
        paragraph: { spacing: { before: 160, after: 80 }, outlineLevel: 1 } },
    ],
  },
  numbering: {
    config: [
      { reference: "bul", levels: [{ level: 0, format: LevelFormat.BULLET, text: "•",
        alignment: AlignmentType.LEFT,
        style: { paragraph: { indent: { left: 460, hanging: 260 } } } }] },
      { reference: "num", levels: [{ level: 0, format: LevelFormat.DECIMAL, text: "%1.",
        alignment: AlignmentType.LEFT,
        style: { paragraph: { indent: { left: 460, hanging: 260 } } } }] },
    ],
  },
  sections: [{
    properties: { page: { margin: { top: 1440, bottom: 1440, left: 1440, right: 1440 } } },
    children,
  }],
});

Packer.toBuffer(doc).then((buf) => {
  fs.writeFileSync(path.join(__dirname, "relatorio.docx"), buf);
  console.log("relatorio.docx gerado (" + buf.length + " bytes)");
});
