/* ============================================
   Mini-Compiler Visualization Dashboard
   JavaScript - Real-time Animation & API
   ============================================ */

// State
let currentPhase = 0;
let isStepMode = false;
let compilationData = null;
let animationSpeed = 5;

// Initialize when DOM is ready
document.addEventListener('DOMContentLoaded', () => {
    initializeApp();
});

function initializeApp() {
    // Get DOM elements
    const codeInput = document.getElementById('codeInput');
    const lineNumbers = document.getElementById('lineNumbers');
    const compileBtn = document.getElementById('compileBtn');
    const stepBtn = document.getElementById('stepBtn');
    const resetBtn = document.getElementById('resetBtn');
    const clearBtn = document.getElementById('clearBtn');
    const speedSlider = document.getElementById('speedSlider');
    const speedValue = document.getElementById('speedValue');

    // Initialize line numbers
    updateLineNumbers();

    // Event listeners
    if (codeInput) {
        codeInput.addEventListener('input', updateLineNumbers);
        codeInput.addEventListener('scroll', () => {
            if (lineNumbers) lineNumbers.scrollTop = codeInput.scrollTop;
        });
    }

    if (compileBtn) {
        compileBtn.addEventListener('click', () => compile(false));
    }

    if (stepBtn) {
        stepBtn.addEventListener('click', () => compile(true));
    }

    if (resetBtn) {
        resetBtn.addEventListener('click', resetVisualization);
    }

    if (clearBtn) {
        clearBtn.addEventListener('click', () => {
            if (codeInput) codeInput.value = '';
            updateLineNumbers();
            resetVisualization();
        });
    }

    if (speedSlider) {
        speedSlider.addEventListener('input', (e) => {
            animationSpeed = parseInt(e.target.value);
            if (speedValue) speedValue.textContent = animationSpeed + 'x';
        });
    }

    // Tab switching
    document.querySelectorAll('.tab-btn').forEach(btn => {
        btn.addEventListener('click', (e) => {
            const tab = e.currentTarget.dataset.tab;
            document.querySelectorAll('.tab-btn').forEach(b => b.classList.remove('active'));
            e.currentTarget.classList.add('active');

            document.getElementById('editorTab').style.display = tab === 'editor' ? 'flex' : 'none';
            document.getElementById('samplesTab').style.display = tab === 'samples' ? 'grid' : 'none';
        });
    });

    // Sample cards
    document.querySelectorAll('.sample-card').forEach(card => {
        card.addEventListener('click', () => {
            loadSample(card.dataset.sample);
            // Switch back to editor
            document.querySelectorAll('.tab-btn').forEach(b => b.classList.remove('active'));
            document.querySelector('.tab-btn[data-tab="editor"]').classList.add('active');
            document.getElementById('editorTab').style.display = 'flex';
            document.getElementById('samplesTab').style.display = 'none';
        });
    });

    // Copy LLVM button
    const copyBtn = document.getElementById('copyLLVM');
    if (copyBtn) {
        copyBtn.addEventListener('click', () => {
            const llvmCode = document.getElementById('llvmCode').textContent;
            navigator.clipboard.writeText(llvmCode);
            copyBtn.innerHTML = '<i class="fas fa-check"></i> Copied!';
            setTimeout(() => {
                copyBtn.innerHTML = '<i class="fas fa-copy"></i> Copy';
            }, 2000);
        });
    }

    console.log('Mini-Compiler Dashboard initialized');
}

// Sample programs
const samplePrograms = {
    basic: `/* Basic Program */
int main() {
    int a = 5;
    int b = 10;
    int sum = a + b;
    return sum;
}`,
    control: `/* Control Flow */
int main() {
    int x = 10;
    int y = 0;

    if (x > 5) {
        y = x * 2;
    } else {
        y = x + 5;
    }

    while (y > 0) {
        y = y - 1;
    }

    return y;
}`,
    function: `/* Functions */
int factorial(int n) {
    int result = 1;
    int i = 1;
    while (i <= n) {
        result = result * i;
        i = i + 1;
    }
    return result;
}

int main() {
    int x = 5;
    int fact = factorial(x);
    return fact;
}`,
    optimize: `/* Optimization Demo */
int main() {
    int a = 2 + 3;
    int b = 10 * 2;
    int x = 5;
    int y = x + 10;
    int p = a + b;
    int q = a + b;

    if (y > 10) {
        y = y * 2;
    }

    int dead = 100;

    return y + p + q;
}`
};

function loadSample(name) {
    const codeInput = document.getElementById('codeInput');
    if (codeInput && samplePrograms[name]) {
        codeInput.value = samplePrograms[name];
        updateLineNumbers();
    }
}

function updateLineNumbers() {
    const codeInput = document.getElementById('codeInput');
    const lineNumbers = document.getElementById('lineNumbers');
    if (!codeInput || !lineNumbers) return;

    const lines = codeInput.value.split('\n').length;
    lineNumbers.innerHTML = Array.from({ length: lines }, (_, i) =>
        `<span>${i + 1}</span>`
    ).join('');
}

function resetVisualization() {
    currentPhase = 0;

    // Reset badges
    for (let i = 1; i <= 6; i++) {
        const badge = document.getElementById(`badge${i}`);
        if (badge) {
            badge.textContent = 'Waiting';
            badge.className = 'phase-badge';
        }
        const navStatus = document.getElementById(`navStatus${i}`);
        if (navStatus) navStatus.className = 'nav-status';
    }

    // Reset panels
    document.querySelectorAll('.phase-panel').forEach(panel => {
        panel.classList.remove('active', 'processing');
    });

    // Clear visualizations
    const tokenStream = document.getElementById('tokenStream');
    if (tokenStream) tokenStream.innerHTML = '<div class="empty-state"><i class="fas fa-coins"></i><p>Tokens will appear here</p></div>';

    const astCanvas = document.getElementById('astCanvas');
    if (astCanvas) astCanvas.innerHTML = '<div class="empty-state"><i class="fas fa-project-diagram"></i><p>AST will be visualized here</p></div>';

    const symbolTableBody = document.getElementById('symbolTableBody');
    if (symbolTableBody) symbolTableBody.innerHTML = '';

    const tacCode = document.getElementById('tacCode');
    if (tacCode) tacCode.innerHTML = '<div class="empty-state"><i class="fas fa-file-code"></i><p>TAC instructions will appear here</p></div>';

    const optBefore = document.getElementById('optBefore');
    if (optBefore) optBefore.innerHTML = '';

    const optAfter = document.getElementById('optAfter');
    if (optAfter) optAfter.innerHTML = '';

    const llvmCode = document.getElementById('llvmCode');
    if (llvmCode) llvmCode.innerHTML = '<div class="empty-state"><i class="fas fa-microchip"></i><p>LLVM IR will appear here</p></div>';

    // Reset stats
    document.getElementById('totalTokens').textContent = '0';
    document.getElementById('totalSymbols').textContent = '0';
    document.getElementById('totalOpts').textContent = '0';
    document.getElementById('tacCount').textContent = '0';
    document.getElementById('tempCount').textContent = '0';
    document.getElementById('labelCount').textContent = '0';

    // Reset compile status
    const compileStatus = document.getElementById('compileStatus');
    if (compileStatus) compileStatus.innerHTML = '<span class="status-text">Ready to compile</span>';
}

// Main compile function
async function compile(stepMode) {
    isStepMode = stepMode;
    resetVisualization();

    const codeInput = document.getElementById('codeInput');
    const code = codeInput ? codeInput.value : '';

    if (!code.trim()) {
        alert('Please enter some code to compile');
        return;
    }

    // Show loading
    const loadingOverlay = document.getElementById('loadingOverlay');
    if (loadingOverlay) loadingOverlay.style.display = 'flex';

    const compileStatus = document.getElementById('compileStatus');
    if (compileStatus) compileStatus.innerHTML = '<span class="status-text">Compiling...</span>';

    try {
        // Try API first, fall back to simulation
        compilationData = await compileWithAPI(code);

        if (loadingOverlay) loadingOverlay.style.display = 'none';

        if (!compilationData.success && compilationData.parseErrors > 0) {
            if (compileStatus) compileStatus.innerHTML = '<span class="status-text error">Parse errors: ' + compilationData.parseErrors + '</span>';
        }

        if (stepMode) {
            await runPhase(1);
        } else {
            await runAllPhases();
        }

        if (compileStatus && compilationData.success) {
            compileStatus.innerHTML = '<span class="status-text success">Compilation successful!</span>';
        }
    } catch (error) {
        console.error('Compilation error:', error);
        if (loadingOverlay) loadingOverlay.style.display = 'none';
        if (compileStatus) compileStatus.innerHTML = '<span class="status-text error">Error: ' + error.message + '</span>';
    }
}

// API call to C backend
async function compileWithAPI(code) {
    console.log('Sending code to C backend API...');
    console.log('Code length:', code.length);

    try {
        const response = await fetch('/api/compile', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/x-www-form-urlencoded',
            },
            body: 'code=' + encodeURIComponent(code)
        });

        console.log('Response status:', response.status);

        if (!response.ok) {
            throw new Error('Server returned ' + response.status);
        }

        const text = await response.text();
        console.log('Raw response:', text.substring(0, 500));

        const data = JSON.parse(text);
        console.log('Parsed API Response:', data);

        // Update status to show API was used
        const compileStatus = document.getElementById('compileStatus');
        if (compileStatus) {
            compileStatus.innerHTML = '<span class="status-text">Using C compiler backend...</span>';
        }

        return {
            tokens: data.tokens || [],
            ast: data.ast || { type: 'Program', children: [] },
            symbols: data.symbols || [],
            tac: data.tac || [],
            optimizedTac: data.optimizedTac || data.tac || [],
            optStats: data.optStats || { constantFolding: 0, constantPropagation: 0, cse: 0, deadCode: 0, total: 0 },
            llvm: data.llvm || [],
            success: data.success,
            parseErrors: data.parseErrors || 0,
            semanticErrors: data.semanticErrors || 0
        };
    } catch (error) {
        console.error('API Error:', error);
        console.log('Falling back to JavaScript simulation');

        // Show warning that simulation is being used
        const compileStatus = document.getElementById('compileStatus');
        if (compileStatus) {
            compileStatus.innerHTML = '<span class="status-text warning">⚠ Using simulation (C server not responding)</span>';
        }

        return simulateCompilation(code);
    }
}

async function runAllPhases() {
    const delay = Math.max(100, 600 - animationSpeed * 50);
    for (let i = 1; i <= 6; i++) {
        await runPhase(i);
        await sleep(delay);
    }
}

async function runPhase(phaseNum) {
    // Mark as processing
    const badge = document.getElementById(`badge${phaseNum}`);
    const navStatus = document.getElementById(`navStatus${phaseNum}`);
    const panel = document.getElementById(`phase${phaseNum}`);

    if (badge) {
        badge.textContent = 'Processing...';
        badge.className = 'phase-badge processing';
    }
    if (navStatus) navStatus.className = 'nav-status processing';
    if (panel) panel.classList.add('processing');

    await sleep(100);

    // Run phase
    try {
        switch (phaseNum) {
            case 1:
                await visualizeTokens(compilationData.tokens);
                break;
            case 2:
                await visualizeAST(compilationData.ast);
                break;
            case 3:
                await visualizeSymbolTable(compilationData.symbols);
                break;
            case 4:
                await visualizeTAC(compilationData.tac);
                break;
            case 5:
                await visualizeOptimization(compilationData.tac, compilationData.optimizedTac, compilationData.optStats);
                break;
            case 6:
                await visualizeLLVM(compilationData.llvm);
                break;
        }

        // Mark as complete
        if (badge) {
            badge.textContent = 'Complete';
            badge.className = 'phase-badge complete';
        }
        if (navStatus) navStatus.className = 'nav-status complete';
        if (panel) {
            panel.classList.remove('processing');
            panel.classList.add('active');
        }
    } catch (error) {
        console.error(`Phase ${phaseNum} error:`, error);
        if (badge) {
            badge.textContent = 'Error';
            badge.className = 'phase-badge error';
        }
    }

    currentPhase = phaseNum;
}

// Phase 1: Token Visualization
async function visualizeTokens(tokens) {
    const container = document.getElementById('tokenStream');
    if (!container) return;

    container.innerHTML = '';

    const delay = Math.max(10, 50 - animationSpeed * 4);

    for (let i = 0; i < tokens.length; i++) {
        const token = tokens[i];
        const tokenEl = document.createElement('div');
        tokenEl.className = `token ${token.category || 'unknown'}`;
        tokenEl.innerHTML = `
            <span class="token-type">${token.type}</span>
            <span class="token-value">${escapeHtml(token.value)}</span>
        `;
        container.appendChild(tokenEl);

        // Animate
        tokenEl.style.opacity = '0';
        tokenEl.style.transform = 'translateY(10px)';
        await sleep(delay);
        tokenEl.style.transition = 'all 0.2s ease';
        tokenEl.style.opacity = '1';
        tokenEl.style.transform = 'translateY(0)';
    }

    // Update stats
    document.getElementById('totalTokens').textContent = tokens.length;

    // Token stats
    const stats = {};
    tokens.forEach(t => {
        stats[t.category] = (stats[t.category] || 0) + 1;
    });

    const tokenStats = document.getElementById('tokenStats');
    if (tokenStats) {
        tokenStats.innerHTML = Object.entries(stats).map(([cat, count]) =>
            `<span class="stat-badge ${cat}">${cat}: ${count}</span>`
        ).join('');
    }
}

// Phase 2: AST Visualization - Tree with connecting lines
async function visualizeAST(ast) {
    const container = document.getElementById('astCanvas');
    if (!container || !ast) return;

    container.innerHTML = '';

    const treeWrapper = document.createElement('div');
    treeWrapper.className = 'tree-wrapper';
    container.appendChild(treeWrapper);

    await renderTreeNode(ast, treeWrapper, 0);
}

async function renderTreeNode(node, container, level) {
    if (!node) return;

    const delay = Math.max(30, 100 - animationSpeed * 8);

    const nodeEl = document.createElement('div');
    nodeEl.className = 'tree-node';

    const content = document.createElement('div');
    content.className = 'node-content';
    content.style.animationDelay = `${level * 0.1}s`;
    content.innerHTML = `
        <span class="node-type">${node.type}</span>
        ${node.value ? `<span class="node-value">${escapeHtml(node.value)}</span>` : ''}
    `;
    nodeEl.appendChild(content);
    container.appendChild(nodeEl);

    await sleep(delay);

    if (node.children && node.children.length > 0) {
        const childrenEl = document.createElement('div');
        childrenEl.className = 'tree-children';
        nodeEl.appendChild(childrenEl);

        for (const child of node.children) {
            if (child) {
                await renderTreeNode(child, childrenEl, level + 1);
            }
        }
    }
}

// Phase 3: Symbol Table Visualization
async function visualizeSymbolTable(symbols) {
    const tbody = document.getElementById('symbolTableBody');
    if (!tbody) return;

    tbody.innerHTML = '';

    const delay = Math.max(30, 100 - animationSpeed * 8);

    for (let i = 0; i < symbols.length; i++) {
        const sym = symbols[i];
        const row = document.createElement('tr');
        row.innerHTML = `
            <td><strong>${sym.name}</strong></td>
            <td><span class="type-badge">${sym.type}</span></td>
            <td>${sym.scope}</td>
            <td>${sym.line || '-'}</td>
            <td>${sym.attributes || '-'}</td>
        `;
        tbody.appendChild(row);

        // Animate
        row.style.opacity = '0';
        await sleep(delay);
        row.style.transition = 'opacity 0.2s ease';
        row.style.opacity = '1';
    }

    document.getElementById('totalSymbols').textContent = symbols.length;
}

// Phase 4: TAC Visualization
async function visualizeTAC(tac) {
    const container = document.getElementById('tacCode');
    if (!container) return;

    container.innerHTML = '';

    const delay = Math.max(10, 40 - animationSpeed * 3);
    let tempCount = 0;
    let labelCount = 0;

    for (let i = 0; i < tac.length; i++) {
        const instr = tac[i];
        const lineEl = document.createElement('div');
        lineEl.className = `tac-line ${instr.isDead ? 'dead' : ''}`;
        lineEl.innerHTML = formatTACInstruction(instr, i);
        container.appendChild(lineEl);

        // Count temps and labels
        if (instr.result && instr.result.startsWith('t')) tempCount++;
        if (instr.op === 'label' || instr.op === 'LABEL') labelCount++;

        // Animate
        lineEl.style.opacity = '0';
        await sleep(delay);
        lineEl.style.transition = 'opacity 0.15s ease';
        lineEl.style.opacity = '1';
    }

    document.getElementById('tacCount').textContent = tac.length;
    document.getElementById('tempCount').textContent = tempCount;
    document.getElementById('labelCount').textContent = labelCount;
}

function formatTACInstruction(instr, index) {
    const lineNum = `<span class="tac-linenum">${String(index).padStart(3, '0')}:</span>`;

    if (instr.op === 'LABEL' || instr.op === 'label' || instr.op === 'FUNC' || instr.op === 'func') {
        return `${lineNum} <span class="tac-label">${instr.result}:</span>`;
    }
    if (instr.op === 'GOTO' || instr.op === 'goto') {
        return `${lineNum} <span class="tac-keyword">goto</span> <span class="tac-label">${instr.label || instr.arg1}</span>`;
    }
    if (instr.op === 'IFFALSE' || instr.op === 'iffalse') {
        return `${lineNum} <span class="tac-keyword">iffalse</span> ${instr.arg1} <span class="tac-keyword">goto</span> <span class="tac-label">${instr.label}</span>`;
    }
    if (instr.op === 'RETURN' || instr.op === 'return') {
        return `${lineNum} <span class="tac-keyword">return</span> ${instr.arg1 || ''}`;
    }
    if (instr.op === 'PARAM' || instr.op === 'param') {
        return `${lineNum} <span class="tac-keyword">param</span> ${instr.arg1}`;
    }
    if (instr.op === 'CALL' || instr.op === 'call') {
        return `${lineNum} ${instr.result} = <span class="tac-keyword">call</span> ${instr.arg1}, ${instr.arg2}`;
    }
    if (instr.op === 'ASSIGN' || instr.op === '=' || instr.op === 'COPY') {
        return `${lineNum} ${instr.result} = ${instr.arg1}`;
    }

    // Binary operation
    if (instr.arg2) {
        return `${lineNum} ${instr.result} = ${instr.arg1} <span class="tac-op">${instr.op}</span> ${instr.arg2}`;
    }

    return `${lineNum} ${instr.result || ''} ${instr.op} ${instr.arg1 || ''}`;
}

// Phase 5: Optimization Visualization
async function visualizeOptimization(before, after, stats) {
    const optBefore = document.getElementById('optBefore');
    const optAfter = document.getElementById('optAfter');

    // Show before
    if (optBefore) {
        optBefore.innerHTML = '';
        for (let i = 0; i < before.length; i++) {
            const line = document.createElement('div');
            line.className = 'tac-line';
            line.innerHTML = formatTACInstruction(before[i], i);
            optBefore.appendChild(line);
        }
    }

    await sleep(300);

    // Show after with highlights
    if (optAfter) {
        optAfter.innerHTML = '';
        for (let i = 0; i < after.length; i++) {
            const line = document.createElement('div');
            line.className = `tac-line ${after[i].isDead ? 'dead' : ''}`;
            line.innerHTML = formatTACInstruction(after[i], i);
            optAfter.appendChild(line);

            await sleep(20);
        }
    }

    // Update optimization stats
    const updateTech = (id, count) => {
        const el = document.getElementById(id);
        if (el) {
            const countEl = el.querySelector('.tech-count');
            if (countEl) countEl.textContent = count;
            if (count > 0) el.classList.add('active');
        }
    };

    updateTech('optCF', stats.constantFolding || 0);
    updateTech('optCP', stats.constantPropagation || 0);
    updateTech('optCSE', stats.cse || 0);
    updateTech('optDCE', stats.deadCode || 0);

    document.getElementById('totalOpts').textContent = stats.total || 0;

    // Calculate reduction
    const reduction = before.length > 0 ? Math.round((1 - after.filter(i => !i.isDead).length / before.length) * 100) : 0;
    const reductionEl = document.getElementById('optReduction');
    if (reductionEl) reductionEl.textContent = reduction + '%';
}

// Phase 6: LLVM Visualization
async function visualizeLLVM(llvm) {
    const container = document.getElementById('llvmCode');
    if (!container) return;

    container.innerHTML = '';

    const delay = Math.max(5, 25 - animationSpeed * 2);

    // If llvm is array of strings
    const lines = Array.isArray(llvm) ? llvm : (llvm.split ? llvm.split('\n') : []);

    for (const line of lines) {
        const lineEl = document.createElement('div');
        lineEl.className = 'llvm-line';
        lineEl.innerHTML = formatLLVMLine(line);
        container.appendChild(lineEl);
        await sleep(delay);
    }
}

function formatLLVMLine(line) {
    if (!line) return '';
    return line
        .replace(/(define|declare|ret|br|call|add|sub|mul|sdiv|icmp|alloca|load|store|sgt|slt|sge|sle|eq|ne|label)/g, '<span class="llvm-keyword">$1</span>')
        .replace(/(i32|i64|i8|i1|double|void|float)/g, '<span class="llvm-type">$1</span>')
        .replace(/(%[\w.]+)/g, '<span class="llvm-register">$1</span>')
        .replace(/(@[\w]+)/g, '<span class="llvm-function">$1</span>')
        .replace(/(;.*)/g, '<span class="llvm-comment">$1</span>');
}

// Simulation fallback
function simulateCompilation(code) {
    return {
        tokens: simulateTokenize(code),
        ast: simulateAST(code),
        symbols: simulateSymbols(code),
        tac: simulateTAC(code),
        optimizedTac: simulateTAC(code),
        optStats: { constantFolding: 2, constantPropagation: 1, cse: 1, deadCode: 1, total: 5 },
        llvm: simulateLLVM(),
        success: true,
        parseErrors: 0,
        semanticErrors: 0
    };
}

function simulateTokenize(code) {
    const tokens = [];
    const keywords = ['int', 'float', 'char', 'void', 'if', 'else', 'while', 'for', 'return'];
    const words = code.match(/\w+|[+\-*\/=<>!&|]+|[{}();,]/g) || [];

    words.forEach((word, i) => {
        let type, category;
        if (keywords.includes(word)) {
            type = word.toUpperCase();
            category = 'keyword';
        } else if (/^\d+$/.test(word)) {
            type = 'INT_LIT';
            category = 'literal';
        } else if (/^[a-zA-Z_]\w*$/.test(word)) {
            type = 'IDENTIFIER';
            category = 'identifier';
        } else if (['+', '-', '*', '/', '=', '<', '>', '==', '!=', '<=', '>='].includes(word)) {
            type = 'OPERATOR';
            category = 'operator';
        } else {
            type = 'DELIMITER';
            category = 'delimiter';
        }
        tokens.push({ type, value: word, category, line: 1, column: i });
    });

    return tokens;
}

function simulateAST(code) {
    // Parse code to generate a more realistic AST
    const ast = { type: 'Program', value: '', children: [] };

    // Find all functions
    const funcRegex = /int\s+(\w+)\s*\([^)]*\)\s*\{/g;
    let funcMatch;

    while ((funcMatch = funcRegex.exec(code)) !== null) {
        const funcName = funcMatch[1];
        const funcNode = {
            type: 'Function',
            value: funcName,
            children: []
        };

        // Add return type
        funcNode.children.push({ type: 'ReturnType', value: 'int', children: [] });

        // Create block with statements
        const blockNode = { type: 'Block', value: '', children: [] };

        // Find variable declarations in this function
        const varRegex = /int\s+(\w+)\s*=\s*([^;]+);/g;
        let varMatch;
        while ((varMatch = varRegex.exec(code)) !== null) {
            const varName = varMatch[1];
            const varExpr = varMatch[2].trim();

            const declNode = {
                type: 'VarDecl',
                value: varName,
                children: []
            };

            // Check if expression is binary operation
            const binMatch = varExpr.match(/(\w+)\s*([+\-*\/])\s*(\w+)/);
            if (binMatch) {
                declNode.children.push({
                    type: 'BinaryOp',
                    value: binMatch[2],
                    children: [
                        { type: 'Operand', value: binMatch[1], children: [] },
                        { type: 'Operand', value: binMatch[3], children: [] }
                    ]
                });
            } else {
                declNode.children.push({ type: 'Literal', value: varExpr, children: [] });
            }

            blockNode.children.push(declNode);
        }

        // Check for if statements
        if (code.includes('if')) {
            blockNode.children.push({
                type: 'IfStmt',
                value: '',
                children: [
                    { type: 'Condition', value: 'y > 10', children: [] },
                    { type: 'ThenBlock', value: '', children: [
                        { type: 'Assignment', value: 'y = y * 2', children: [] }
                    ]}
                ]
            });
        }

        // Check for while statements
        if (code.includes('while')) {
            blockNode.children.push({
                type: 'WhileStmt',
                value: '',
                children: [
                    { type: 'Condition', value: 'i <= n', children: [] },
                    { type: 'LoopBody', value: '', children: [] }
                ]
            });
        }

        // Add return statement
        const returnMatch = code.match(/return\s+([^;]+);/);
        if (returnMatch) {
            blockNode.children.push({
                type: 'Return',
                value: returnMatch[1],
                children: []
            });
        }

        funcNode.children.push(blockNode);
        ast.children.push(funcNode);
    }

    return ast;
}

function simulateSymbols(code) {
    const symbols = [];
    const varMatches = code.match(/int\s+(\w+)/g) || [];

    varMatches.forEach((v, i) => {
        const name = v.replace('int ', '');
        symbols.push({
            name,
            type: 'int',
            scope: i === 0 ? 0 : 1,
            line: i + 1,
            attributes: i === 0 && name !== 'main' ? 'function' : ''
        });
    });

    return symbols;
}

function simulateTAC(code) {
    return [
        { op: 'FUNC', result: 'main', arg1: '', arg2: '' },
        { op: 'ADD', result: 't0', arg1: '2', arg2: '3' },
        { op: 'ASSIGN', result: 'a', arg1: 't0', arg2: '' },
        { op: 'MUL', result: 't1', arg1: '10', arg2: '2' },
        { op: 'ASSIGN', result: 'b', arg1: 't1', arg2: '' },
        { op: 'RETURN', result: '', arg1: 'a', arg2: '' }
    ];
}

function simulateLLVM() {
    return [
        '; LLVM IR generated by Mini-Compiler',
        '',
        'define i32 @main() {',
        'entry:',
        '  %a = add i32 2, 3',
        '  %b = mul i32 10, 2',
        '  ret i32 %a',
        '}'
    ];
}

// Utility functions
function sleep(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
}

function escapeHtml(text) {
    if (!text) return '';
    const div = document.createElement('div');
    div.textContent = text;
    return div.innerHTML;
}

// Keyboard shortcuts
document.addEventListener('keydown', (e) => {
    // Ctrl+Enter to compile
    if (e.ctrlKey && e.key === 'Enter') {
        compile(false);
    }
    // Space in step mode
    if (e.key === ' ' && isStepMode && currentPhase < 6) {
        e.preventDefault();
        runPhase(currentPhase + 1);
    }
});
