const seedTransactions = [
 {id:1,date:'2026-07-08',symbol:'TCS',type:'BUY',qty:12,price:3320,fees:20},
 {id:2,date:'2026-07-18',symbol:'INFY',type:'BUY',qty:20,price:1480,fees:18},
 {id:3,date:'2026-08-02',symbol:'RELIANCE',type:'BUY',qty:8,price:1260,fees:16},
 {id:4,date:'2026-08-15',symbol:'TCS',type:'BUY',qty:5,price:3410,fees:12},
 {id:5,date:'2026-09-04',symbol:'HDFCBANK',type:'BUY',qty:15,price:1590,fees:20},
 {id:6,date:'2026-09-20',symbol:'INFY',type:'BUY',qty:8,price:1510,fees:10},
 {id:7,date:'2026-10-04',symbol:'RELIANCE',type:'BUY',qty:4,price:1300,fees:10},
 {id:8,date:'2026-10-14',symbol:'TCS',type:'DIVIDEND',qty:17,price:11,fees:0}
];

const seedPrices = {
 TCS:3568.4,
 INFY:1582.7,
 RELIANCE:1398.2,
 HDFCBANK:1712.5,
 ICICIBANK:1285.3
};

const companies = {
 TCS:'Tata Consultancy Services',
 INFY:'Infosys',
 RELIANCE:'Reliance Industries',
 HDFCBANK:'HDFC Bank',
 ICICIBANK:'ICICI Bank'
};

let transactions = JSON.parse(localStorage.getItem('ed_txns') || 'null') || seedTransactions;
let prices = JSON.parse(localStorage.getItem('ed_prices') || 'null') || seedPrices;
let journals = JSON.parse(localStorage.getItem('ed_journal') || 'null') || [
 {symbol:'TCS',reason:'LONG_TERM',confidence:4,note:'Accumulating gradually based on long-term business conviction.',date:'2026-10-14'},
 {symbol:'INFY',reason:'VALUATION',confidence:3,note:'Watching valuation and margin commentary before adding more.',date:'2026-10-21'}
];

let range = '1M';

const money = n => '₹' + Number(n || 0).toLocaleString('en-IN',{
 minimumFractionDigits:2,
 maximumFractionDigits:2
});

const fmt = n => Number(n || 0).toLocaleString('en-IN',{
 maximumFractionDigits:2
});

const dateFmt = s => new Date(s + 'T00:00:00').toLocaleDateString('en-IN',{
 day:'2-digit',
 month:'short',
 year:'numeric'
});

function persist(){
 localStorage.setItem('ed_txns',JSON.stringify(transactions));
 localStorage.setItem('ed_prices',JSON.stringify(prices));
 localStorage.setItem('ed_journal',JSON.stringify(journals));
}

function toast(msg){
 const t = document.getElementById('toast');
 t.textContent = msg;
 t.classList.add('show');
 setTimeout(() => t.classList.remove('show'),2600);
}

function calculate(asOf='9999-12-31'){
 const map = {};

 const tx = transactions
  .filter(t => t.date <= asOf)
  .slice()
  .sort((a,b) => a.date.localeCompare(b.date) || a.id-b.id);

 tx.forEach(t => {
  const h = map[t.symbol] || (map[t.symbol] = {
   symbol:t.symbol,
   qty:0,
   cost:0,
   realised:0,
   dividend:0
  });

  if(t.type === 'BUY'){
   h.qty += t.qty;
   h.cost += t.qty * t.price + t.fees;
  }
  else if(t.type === 'SELL'){
   const q = Math.min(t.qty,h.qty);
   const avg = h.qty ? h.cost / h.qty : 0;
   h.realised += q * t.price - t.fees - avg * q;
   h.cost -= avg * q;
   h.qty -= q;
  }
  else if(t.type === 'DIVIDEND'){
   h.dividend += t.qty * t.price;
  }
 });

 return Object.values(map)
  .filter(h => h.qty > 0)
  .map(h => ({
   ...h,
   avg:h.qty ? h.cost / h.qty : 0,
   ltp:prices[h.symbol] || 0,
   value:h.qty * (prices[h.symbol] || 0),
   unreal:h.qty * (prices[h.symbol] || 0) - h.cost
  }));
}

function totals(){
 const hs = calculate();

 return {
  hs,
  invested:hs.reduce((s,h) => s+h.cost,0),
  value:hs.reduce((s,h) => s+h.value,0),
  unreal:hs.reduce((s,h) => s+h.unreal,0),
  realised:hs.reduce((s,h) => s+h.realised,0),
  dividend:hs.reduce((s,h) => s+h.dividend,0)
 };
}

function metric(label,value,foot,icon,cls=''){
 return `<div class="card metric">
  <div class="metric-top"><span>${label}</span><span class="metric-icon">${icon}</span></div>
  <div class="metric-value ${cls}">${value}</div>
  <div class="metric-foot">${foot}</div>
 </div>`;
}

function renderMetrics(){
 const t = totals();
 const ret = t.value + t.realised + t.dividend - t.invested;
 const retPct = t.invested ? ret / t.invested * 100 : 0;

 document.getElementById('metricCards').innerHTML =
  metric('Total portfolio value',money(t.value),'Across '+t.hs.length+' active positions','◫') +
  metric('Invested amount',money(t.invested),'Current open cost basis','↗') +
  metric('Total return',money(ret),(retPct>=0?'+':'')+retPct.toFixed(2)+'% overall return','⌁',ret>=0?'positive':'negative') +
  metric('Unrealised P&L',(t.unreal>=0?'+':'')+money(t.unreal),'On your open positions','◈',t.unreal>=0?'positive':'negative');

 document.getElementById('holdingMetrics').innerHTML =
  metric('Open positions',t.hs.length,'Stocks currently held','▤') +
  metric('Invested capital',money(t.invested),'Open cost basis','↗') +
  metric('Market value',money(t.value),'At latest demo prices','◫') +
  metric('Unrealised P&L',(t.unreal>=0?'+':'')+money(t.unreal),'Open positions only','⌁',t.unreal>=0?'positive':'negative');

 document.getElementById('analyticsMetrics').innerHTML =
  metric('Realised P&L',money(t.realised),'Closed positions','↗',t.realised>=0?'positive':'negative') +
  metric('Dividend income',money(t.dividend),'Recorded dividends','₹','positive') +
  metric('Unrealised P&L',money(t.unreal),'Open positions','⌁',t.unreal>=0?'positive':'negative') +
  metric('Total return',money(ret),retPct.toFixed(2)+'% on invested capital','◈',ret>=0?'positive':'negative');
}

function holdingRow(h,full=false){
 return `<tr>
  <td>
   <span class="symbol-badge">${h.symbol.slice(0,2)}</span>
   <span class="stock-name">${h.symbol}</span>
   <span class="stock-sub">${companies[h.symbol] || 'Listed equity'}</span>
  </td>
  <td>${fmt(h.qty)}</td>
  <td>${money(h.avg)}</td>
  <td>${money(h.ltp)}</td>
  ${full ? `<td>${money(h.cost)}</td>` : ''}
  <td>${money(h.value)}</td>
  <td class="${h.unreal>=0?'positive':'negative'}">${h.unreal>=0?'+':''}${money(h.unreal)}</td>
 </tr>`;
}

function renderHoldings(){
 const hs = totals().hs.slice().sort((a,b) => b.value-a.value);

 document.getElementById('topHoldings').innerHTML =
  hs.slice(0,4).map(h => holdingRow(h)).join('') ||
  '<tr><td colspan="6" class="empty">No holdings yet. Add a buy transaction to get started.</td></tr>';

 document.getElementById('allHoldings').innerHTML =
  hs.map(h => holdingRow(h,true)).join('') ||
  '<tr><td colspan="7" class="empty">No holdings yet. Add a transaction to create your first position.</td></tr>';

 const legend = document.getElementById('legend');
 const donut = document.getElementById('donut');
 const colors = ['#4f8cff','#22c58b','#f4b65f','#a78bfa','#ed7a91'];
 const total = hs.reduce((s,h) => s+h.value,0) || 1;
 const circ = 2 * Math.PI * 55;
 let offset = 0;

 donut.innerHTML =
  `<circle cx="80" cy="80" r="55" fill="none" stroke="light-dark(#edf1f7,#1b2940)" stroke-width="18"/>` +
  hs.map((h,i) => {
   const len = h.value / total * circ;
   const el = `<circle cx="80" cy="80" r="55" fill="none" stroke="${colors[i%colors.length]}" stroke-width="18" stroke-dasharray="${len} ${circ-len}" stroke-dashoffset="${-offset}" transform="rotate(-90 80 80)"/>`;
   offset += len;
   return el;
  }).join('') +
  `<text x="80" y="76" fill="currentColor" text-anchor="middle" font-size="9">PORTFOLIO</text>
   <text x="80" y="91" fill="currentColor" text-anchor="middle" font-size="12" font-weight="700">${hs.length} stocks</text>`;

 legend.innerHTML = hs.map((h,i) =>
  `<div class="legend-row">
   <span class="legend-name"><i class="swatch" style="background:${colors[i%colors.length]}"></i>${h.symbol}</span>
   <b>${(h.value/total*100).toFixed(1)}%</b>
  </div>`
 ).join('') || '<span class="muted">No allocation yet</span>';

 document.getElementById('performanceBars').innerHTML = hs.map(h =>
  `<div style="margin:16px 0">
   <div style="display:flex;justify-content:space-between;font-size:11px;margin-bottom:7px">
    <b>${h.symbol}</b>
    <span class="${h.unreal>=0?'positive':'negative'}">${h.unreal>=0?'+':''}${money(h.unreal)}</span>
   </div>
   <div style="height:7px;border-radius:10px;background:light-dark(#edf1f7,#1c2a40);overflow:hidden">
    <div style="height:100%;width:${Math.min(100,Math.abs(h.unreal)/(h.cost||1)*100)}%;background:${h.unreal>=0?'#20c58a':'#ed7777'};border-radius:10px"></div>
   </div>
  </div>`
 ).join('') || '<p class="muted">No positions to analyse.</p>';

 document.getElementById('breakdown').innerHTML =
  hs.map(h => `<div class="statline"><span>${h.symbol}</span><b>${money(h.value)}</b></div>`).join('');
}

function renderTransactions(){
 let list = transactions.slice().sort((a,b) => b.date.localeCompare(a.date) || b.id-a.id);
 const filter = document.getElementById('txnFilter').value;

 if(filter !== 'ALL'){
  list = list.filter(t => t.type === filter);
 }

 document.getElementById('txnCount').textContent = transactions.length + ' recorded entries';

 document.getElementById('transactionsTable').innerHTML =
  list.map(t => `<tr>
   <td>${dateFmt(t.date)}</td>
   <td><b>${t.symbol}</b></td>
   <td><span class="tag ${t.type==='SELL'?'warn':t.type==='DIVIDEND'?'blue':''}">${t.type}</span></td>
   <td>${fmt(t.qty)}</td>
   <td>${money(t.price)}</td>
   <td>${money(t.fees)}</td>
   <td>${money(t.qty*t.price+t.fees)}</td>
  </tr>`).join('') ||
  '<tr><td colspan="7" class="empty">No transactions match this filter.</td></tr>';

 document.getElementById('recentActivity').innerHTML =
  list.slice(0,4).map(t => `<div class="activity-row">
   <div class="activity-icon">${t.type==='BUY'?'↗':t.type==='SELL'?'↙':'₹'}</div>
   <div class="activity-info"><b>${t.type} · ${t.symbol}</b><small>${dateFmt(t.date)} · ${fmt(t.qty)} shares</small></div>
   <div class="activity-amount">${money(t.qty*t.price)}</div>
  </div>`).join('') || '<div class="empty">No recent activity.</div>';
}

function drawChart(){
 const svg = document.getElementById('performanceChart');
 const n = range==='1M' ? 9 : range==='3M' ? 13 : range==='6M' ? 17 : range==='1Y' ? 21 : 25;
 const t = totals();
 const base = Math.max(1000,t.invested*.86);
 const end = t.value;

 const vals = Array.from({length:n},(_,i) =>
  base + (end-base)*i/(n-1) +
  Math.sin(i*1.7)*Math.max(300,end*.012) +
  Math.cos(i*.8)*Math.max(150,end*.006)
 );

 vals[n-1] = end;

 const min = Math.min(...vals)*.96;
 const max = Math.max(...vals)*1.04;
 const pts = vals.map((v,i) => [
  18+i*664/(n-1),
  195-(v-min)/(max-min||1)*165
 ]);

 const line = pts.map((p,i) =>
  (i?'L':'M')+p[0].toFixed(1)+','+p[1].toFixed(1)
 ).join(' ');

 const area = line + ` L ${pts[pts.length-1][0]} 200 L 18 200 Z`;

 svg.innerHTML =
  `<defs><linearGradient id="area" x1="0" x2="0" y1="0" y2="1">
   <stop offset="0" stop-color="#4f8cff" stop-opacity=".28"/>
   <stop offset="1" stop-color="#4f8cff" stop-opacity="0"/>
  </linearGradient></defs>` +
  [35,85,135,185].map(y =>
   `<line class="chart-grid" x1="12" y1="${y}" x2="690" y2="${y}"/>`
  ).join('') +
  `<path class="chart-area" d="${area}"/><path class="chart-line" d="${line}"/>` +
  ['Jul','Aug','Sep','Oct','Nov'].map((m,i) =>
   `<text class="chart-label" x="${20+i*164}" y="218">${m}</text>`
  ).join('') +
  `<circle cx="${pts[n-1][0]}" cy="${pts[n-1][1]}" r="4" fill="#4f8cff" stroke="light-dark(#fff,#0e1727)" stroke-width="2"/>`;
}

function renderJournal(){
 document.getElementById('journalList').innerHTML =
  journals.slice().reverse().map(j =>
   `<div class="activity-row" style="align-items:flex-start">
    <div class="activity-icon">✎</div>
    <div class="activity-info">
     <b>${j.symbol} <span class="tag blue">${j.reason}</span></b>
     <small>${dateFmt(j.date)} · Confidence ${j.confidence}/5</small>
     <div style="font-size:11px;line-height:1.6;margin-top:7px;color:light-dark(#59677e,#bdc9dc)">${escapeHTML(j.note)}</div>
    </div>
   </div>`
  ).join('') || '<div class="empty">No journal notes yet.</div>';
}

function escapeHTML(s){
 return String(s).replace(/[&<>"']/g,c => ({
  '&':'&amp;',
  '<':'&lt;',
  '>':'&gt;',
  '"':'&quot;',
  "'":'&#39;'
 }[c]));
}

function render(){
 renderMetrics();
 renderHoldings();
 renderTransactions();
 renderJournal();
 drawChart();

 const hs = totals().hs;

 document.getElementById('simSymbol').innerHTML =
  hs.map(h => `<option value="${h.symbol}">${h.symbol} · ${fmt(h.qty)} shares</option>`).join('') ||
  '<option value="">No holdings</option>';

 if(hs.length){
  document.getElementById('simPrice').value = prices[hs[0].symbol].toFixed(2);
 }

 document.getElementById('today').textContent =
  new Date().toLocaleDateString('en-IN',{
   weekday:'short',
   day:'numeric',
   month:'short',
   year:'numeric'
  });
}

function showPage(name){
 document.querySelectorAll('.page').forEach(p =>
  p.classList.toggle('active',p.id==='page-'+name)
 );

 document.querySelectorAll('.nav button').forEach(b =>
  b.classList.toggle('active',b.dataset.page===name)
 );

 const labels = {
  overview:'Overview',
  holdings:'My holdings',
  transactions:'Transactions',
  analytics:'Analytics',
  simulator:'What-if simulator',
  journal:'Decision journal',
  timetravel:'Time travel'
 };

 document.getElementById('crumb').textContent = labels[name] || 'Overview';
 document.getElementById('sidebar').classList.remove('open');

 if(name==='simulator'){
  render();

  const symbol = document.getElementById('simSymbol').value;
  const h = calculate().find(x => x.symbol===symbol);

  if(h){
   document.getElementById('simQty').value = Math.min(1,h.qty);
   document.getElementById('simPrice').value = h.ltp.toFixed(2);
  }
 }
}

/* FIX: Listen to BOTH sidebar navigation sections. */
document.querySelectorAll('.nav').forEach(nav => {
 nav.addEventListener('click', e => {
  const b = e.target.closest('[data-page]');

  if(b && nav.contains(b)){
   showPage(b.dataset.page);
  }
 });
});

document.querySelectorAll('[data-goto]').forEach(b =>
 b.addEventListener('click',() => showPage(b.dataset.goto))
);

document.getElementById('mobileMenu').onclick = () =>
 document.getElementById('sidebar').classList.toggle('open');

document.querySelectorAll('[data-open-add]').forEach(b => {
 b.onclick = () => {
  document.getElementById('fDate').value = new Date().toISOString().slice(0,10);
  document.getElementById('txnModal').classList.add('show');
 };
});

document.getElementById('cancelModal').onclick = () =>
 document.getElementById('txnModal').classList.remove('show');

document.getElementById('txnFilter').onchange = renderTransactions;

document.querySelectorAll('[data-range]').forEach(b => {
 b.onclick = () => {
  document.querySelectorAll('[data-range]').forEach(x => x.classList.remove('active'));
  b.classList.add('active');
  range = b.dataset.range;
  drawChart();
 };
});

document.getElementById('saveTxn').onclick = () => {
 const type = document.getElementById('fType').value;
 const symbol = document.getElementById('fSymbol').value.trim().toUpperCase();
 const date = document.getElementById('fDate').value;
 const qty = Number(document.getElementById('fQty').value);
 const price = Number(document.getElementById('fPrice').value);
 const fees = Number(document.getElementById('fFees').value);

 if(!/^[A-Z0-9.-]{1,15}$/.test(symbol) || !date ||
    !Number.isFinite(qty) || qty<=0 ||
    !Number.isFinite(price) || price<=0 ||
    !Number.isFinite(fees) || fees<0){
  toast('Please enter valid transaction details.');
  return;
 }

 if(type==='SELL'){
  const h = calculate().find(x => x.symbol===symbol);
  if(!h || h.qty<qty){
   toast('Not enough shares available to sell.');
   return;
  }
 }

 transactions.push({
  id:Math.max(0,...transactions.map(t => t.id))+1,
  date,
  symbol,
  type,
  qty,
  price,
  fees
 });

 if(type==='BUY' && !prices[symbol]){
  prices[symbol] = price;
 }

 persist();
 render();
 document.getElementById('txnModal').classList.remove('show');
 toast('Transaction saved to this browser.');
};

document.getElementById('priceUpdate').onclick = () => {
 Object.keys(prices).forEach((s,i) => {
  prices[s] = Math.round(prices[s] * (1+(i%2?.004:-.003)) * 100) / 100;
 });
 persist();
 render();
 toast('Illustrative demo prices updated.');
};

document.getElementById('runSimulation').onclick = () => {
 const symbol = document.getElementById('simSymbol').value;
 const h = calculate().find(x => x.symbol===symbol);
 const qty = Number(document.getElementById('simQty').value);
 const price = Number(document.getElementById('simPrice').value);
 const fees = Number(document.getElementById('simFees').value);
 const out = document.getElementById('simResult');

 if(!h || qty<=0 || qty>h.qty || price<=0 || fees<0){
  out.innerHTML = '<span class="negative">Enter a valid quantity, price and fee. Quantity cannot exceed your holding.</span>';
  return;
 }

 const cost = h.avg * qty;
 const proceeds = qty * price - fees;
 const pnl = proceeds - cost;

 out.innerHTML = `
  <div class="statline"><span>Estimated cost basis</span><b>${money(cost)}</b></div>
  <div class="statline"><span>Net sale proceeds</span><b>${money(proceeds)}</b></div>
  <div class="statline"><span>Estimated realised P&amp;L</span><b class="${pnl>=0?'positive':'negative'}">${pnl>=0?'+':''}${money(pnl)}</b></div>
  <div class="statline"><span>Return on cost</span><b>${cost ? (pnl/cost*100).toFixed(2) : '0.00'}%</b></div>
  <p class="disclaimer">Simulation only. Your actual holdings and transaction history have not been changed.</p>
 `;
};

document.getElementById('addJournal').onclick = () =>
 document.getElementById('journalModal').classList.add('show');

document.getElementById('cancelJournal').onclick = () =>
 document.getElementById('journalModal').classList.remove('show');

document.getElementById('saveJournal').onclick = () => {
 const symbol = document.getElementById('jSymbol').value.trim().toUpperCase();
 const note = document.getElementById('jNote').value.trim();

 if(!symbol || !note){
  toast('Enter a stock symbol and a note.');
  return;
 }

 journals.push({
  symbol,
  reason:document.getElementById('jReason').value,
  confidence:Number(document.getElementById('jConfidence').value),
  note,
  date:new Date().toISOString().slice(0,10)
 });

 persist();
 renderJournal();
 document.getElementById('journalModal').classList.remove('show');
 toast('Journal entry saved.');
};

document.getElementById('travelBtn').onclick = () => {
 const date = document.getElementById('travelDate').value;

 if(!date){
  toast('Choose a date first.');
  return;
 }

 const hs = calculate(date);
 const value = hs.reduce((s,h) => s+h.value,0);
 const cost = hs.reduce((s,h) => s+h.cost,0);

 document.getElementById('travelResult').innerHTML = `
  <div class="statline"><span>Open positions</span><b>${hs.length}</b></div>
  <div class="statline"><span>Invested cost basis</span><b>${money(cost)}</b></div>
  <div class="statline"><span>Value at current demo prices</span><b>${money(value)}</b></div>
  <div class="statline"><span>Unrealised P&amp;L</span><b class="${value-cost>=0?'positive':'negative'}">${money(value-cost)}</b></div>
  <div style="margin-top:12px;font-size:11px;font-weight:700">Holdings on ${dateFmt(date)}</div>
  ${hs.map(h => `<div class="statline"><span>${h.symbol} · ${h.qty} shares</span><b>${money(h.value)}</b></div>`).join('') || '<p class="muted">No open positions on this date.</p>'}
  <p class="disclaimer">Transactions after the selected date were excluded. Prices are current demo prices, not historical closing prices.</p>
 `;
};

document.getElementById('exportBtn').onclick = () => {
 const rows = [
  ['Date','Symbol','Type','Quantity','Price INR','Fees INR'],
  ...transactions.map(t => [t.date,t.symbol,t.type,t.qty,t.price,t.fees])
 ];

 const csv = rows.map(r => r.join(',')).join('\n');
 const blob = new Blob([csv],{type:'text/csv'});
 const url = URL.createObjectURL(blob);
 const a = document.createElement('a');

 a.href = url;
 a.download = 'equitydesk-transactions.csv';
 a.click();

 URL.revokeObjectURL(url);
 toast('Transaction CSV exported.');
};

document.getElementById('fType').onchange = e => {
 document.getElementById('fPrice').placeholder =
  e.target.value==='DIVIDEND' ? 'Dividend per share' : '';
};

document.getElementById('travelDate').value = '2026-09-30';

render();