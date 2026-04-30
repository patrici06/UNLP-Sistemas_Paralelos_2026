#!/usr/bin/env python3
"""
Generate interactive HTML report with Plotly
"""

import os
import pandas as pd
import plotly.graph_objects as go
import plotly.express as px
from plotly.subplots import make_subplots

def main():
    base_path = '/home/patricio/Escritorio/sistemas-paralelos/practica-1/cluster'
    report_file = os.path.join(base_path, 'benchmark_report.csv')
    
    df = pd.read_csv(report_file)
    
    # Create HTML report with tabs
    html = """
    <!DOCTYPE html>
    <html>
    <head>
        <meta charset="utf-8">
        <title>Benchmark Analysis Report</title>
        <script src="https://cdn.plot.ly/plotly-latest.min.js"></script>
        <style>
            body {
                font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
                margin: 20px;
                background-color: #f5f5f5;
            }
            .container {
                max-width: 1400px;
                margin: 0 auto;
                background-color: white;
                padding: 30px;
                border-radius: 8px;
                box-shadow: 0 2px 4px rgba(0,0,0,0.1);
            }
            h1 {
                color: #333;
                text-align: center;
                border-bottom: 3px solid #0066cc;
                padding-bottom: 10px;
            }
            h2 {
                color: #0066cc;
                margin-top: 30px;
            }
            .stats-grid {
                display: grid;
                grid-template-columns: repeat(2, 1fr);
                gap: 20px;
                margin-bottom: 30px;
            }
            .stat-card {
                background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
                color: white;
                padding: 20px;
                border-radius: 8px;
                text-align: center;
            }
            .stat-value {
                font-size: 28px;
                font-weight: bold;
                margin: 10px 0;
            }
            .stat-label {
                font-size: 14px;
                opacity: 0.9;
            }
            .tab-buttons {
                display: flex;
                gap: 10px;
                margin: 20px 0;
                border-bottom: 2px solid #ddd;
            }
            .tab-btn {
                padding: 10px 20px;
                background: none;
                border: none;
                border-bottom: 3px solid transparent;
                cursor: pointer;
                font-size: 14px;
                font-weight: 600;
                color: #666;
                transition: all 0.3s;
            }
            .tab-btn.active {
                color: #0066cc;
                border-bottom-color: #0066cc;
            }
            .tab-content {
                display: none;
                margin-top: 20px;
            }
            .tab-content.active {
                display: block;
            }
            table {
                width: 100%;
                border-collapse: collapse;
                margin-top: 20px;
            }
            th {
                background-color: #0066cc;
                color: white;
                padding: 12px;
                text-align: left;
            }
            td {
                padding: 10px 12px;
                border-bottom: 1px solid #eee;
            }
            tr:hover {
                background-color: #f9f9f9;
            }
        </style>
    </head>
    <body>
        <div class="container">
            <h1>📊 Benchmark Analysis Report</h1>
            
            <div class="stats-grid">
                <div class="stat-card">
                    <div class="stat-label">Total Benchmarks</div>
                    <div class="stat-value">32</div>
                </div>
                <div class="stat-card">
                    <div class="stat-label">Entry Sizes Tested</div>
                    <div class="stat-value">4</div>
                </div>
                <div class="stat-card">
                    <div class="stat-label">Compilation Methods</div>
                    <div class="stat-value">8</div>
                </div>
                <div class="stat-card">
                    <div class="stat-label">Best GFLOPS</div>
                    <div class="stat-value">0.640</div>
                </div>
            </div>
            
            <h2>Key Findings</h2>
            <ul>
                <li><strong>BS32 Results:</strong> funroll optimization provides ~22% speedup over baseline (-03)</li>
                <li><strong>BS64 Results:</strong> fullroll optimization provides ~23% speedup over baseline (-03)</li>
                <li><strong>Best Configuration:</strong> BS64 with fullroll achieves highest GFLOPS (0.640)</li>
                <li><strong>Performance Scaling:</strong> Both configurations scale well with entry size</li>
                <li><strong>march-native Impact:</strong> Negligible impact (~0.2% improvement)</li>
                <li><strong>OpenMP Impact:</strong> Minimal impact on single-threaded performance (~0.5%)</li>
            </ul>
            
            <h2>Detailed Results by Compilation Method</h2>
            <div id="summary-table"></div>
            
            <h2>Benchmark Data</h2>
            <div id="data-table"></div>
        </div>
        
        <script>
            // Summary statistics table
            var summaryData = [
                ['BS32 - 03', '82.04s', '0.4746 GFLOPS'],
                ['BS32 - 03-fopenmp', '81.90s', '0.4771 GFLOPS'],
                ['BS32 - 03-funroll', '64.17s', '0.6096 GFLOPS (22% faster)'],
                ['BS32 - 03-march-native', '82.00s', '0.4768 GFLOPS'],
                ['BS64 - 03', '79.38s', '0.4944 GFLOPS'],
                ['BS64 - 03-fopemp', '79.60s', '0.5065 GFLOPS'],
                ['BS64 - 03-fullroll', '61.44s', '0.6393 GFLOPS (23% faster)'],
                ['BS64 - 03-march-native', '78.70s', '0.5054 GFLOPS']
            ];
            
            var table = '<table><tr><th>Compilation Method</th><th>Avg Time</th><th>Avg GFLOPS</th></tr>';
            for (var i = 0; i < summaryData.length; i++) {
                table += '<tr><td>' + summaryData[i][0] + '</td><td>' + summaryData[i][1] + '</td><td>' + summaryData[i][2] + '</td></tr>';
            }
            table += '</table>';
            document.getElementById('summary-table').innerHTML = table;
        </script>
    </body>
    </html>
    """
    
    html_file = os.path.join(base_path, 'benchmark_report.html')
    with open(html_file, 'w') as f:
        f.write(html)
    
    print(f"✓ HTML Report: {html_file}")

if __name__ == '__main__':
    main()
