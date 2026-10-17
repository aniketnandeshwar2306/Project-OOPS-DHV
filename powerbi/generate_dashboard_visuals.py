"""
CitySync / CivicCare — Power BI Dashboard Visual Generator
Generates high-resolution dashboard mockups and visual analytics exports matching
the exact Power BI report layout, color palette, and visual specifications.
"""

import os
import sys
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.patches as patches
import seaborn as sns

# Set publication style
plt.style.use('seaborn-v0_8-whitegrid')
plt.rcParams.update({
    'font.family': 'sans-serif',
    'font.sans-serif': ['Segoe UI', 'DejaVu Sans', 'Arial'],
    'axes.edgecolor': '#E0E0E0',
    'axes.linewidth': 0.8,
    'grid.color': '#F0F0F0'
})

DATA_DIR = "powerbi/data"
VISUALS_DIR = "powerbi/visuals"
os.makedirs(VISUALS_DIR, exist_ok=True)

# Corporate Municipal Palette
NAVY = "#0B3C5D"
SKY = "#328CC1"
GOLD = "#D9B310"
EMERALD = "#1E824C"
AMBER = "#E68A00"
CRIMSON = "#D9381E"
SLATE = "#4F5D75"
LIGHT_BG = "#F7F9FB"

def generate_all_visuals():
    print("=" * 80)
    print("GENERATING POWER BI DASHBOARD VISUAL ANALYTICS EXPORTS")
    print("=" * 80)

    # Load curated data
    fact_df = pd.read_csv(os.path.join(DATA_DIR, "fact_grievances.csv"), nrows=50000)
    dim_cat = pd.read_csv(os.path.join(DATA_DIR, "dim_category.csv"))
    dim_ward = pd.read_csv(os.path.join(DATA_DIR, "dim_ward.csv"))
    funnel_df = pd.read_csv(os.path.join(DATA_DIR, "fact_lifecycle_funnel.csv"))
    heatmap_df = pd.read_csv(os.path.join(DATA_DIR, "fact_hourly_heatmap.csv"))

    # ==========================================================================
    # VISUAL 1: EXECUTIVE OVERVIEW DASHBOARD
    # ==========================================================================
    print("[1/5] Generating Page 1: Executive KPI Overview & Inflow...")
    fig = plt.figure(figsize=(18, 10), facecolor=LIGHT_BG)
    gs = fig.add_gridspec(3, 3, height_ratios=[0.8, 2.2, 2.2], hspace=0.35, wspace=0.25)

    # Top KPI Header Cards
    kpis = [
        ("TOTAL COMPLAINTS", "1,207,746", "Multi-channel total", NAVY),
        ("RESOLUTION RATE", "90.8%", "1.09M tickets resolved", EMERALD),
        ("MEDIAN TURNAROUND", "6.6 Hours", "Resilient to tail delays", SKY),
        ("SLA COMPLIANCE", "82.4%", "Within Citizen Charter", EMERALD),
        ("FAKE CLOSURE RATE", "9.99%", "120,710 flagged anomalies", CRIMSON)
    ]

    card_ax = fig.add_subplot(gs[0, :])
    card_ax.axis('off')
    for i, (title, val, sub, col) in enumerate(kpis):
        x = i * 0.20 + 0.01
        card_rect = patches.FancyBboxPatch((x, 0.05), 0.18, 0.88, boxstyle="round,pad=0.03", 
                                          facecolor="white", edgecolor="#E2E8F0", linewidth=1.2, transform=card_ax.transAxes)
        card_ax.add_patch(card_rect)
        card_ax.text(x + 0.09, 0.68, title, ha='center', va='center', fontsize=9, fontweight='bold', color=SLATE, transform=card_ax.transAxes)
        card_ax.text(x + 0.09, 0.40, val, ha='center', va='center', fontsize=16, fontweight='bold', color=col, transform=card_ax.transAxes)
        card_ax.text(x + 0.09, 0.16, sub, ha='center', va='center', fontsize=7.5, color='#718096', transform=card_ax.transAxes)

    # Visual 1A: Complaint Distribution across Categories (Donut)
    ax1 = fig.add_subplot(gs[1, 0])
    cat_counts = fact_df['standard_category'].value_counts()
    colors = [NAVY, SKY, EMERALD, GOLD, SLATE, "#8E44AD", "#E67E22", "#16A085", "#2C3E50"]
    wedges, texts = ax1.pie(cat_counts.values, startangle=140, colors=colors, wedgeprops=dict(width=0.45, edgecolor='white'))
    ax1.set_title("Grievance Volume by Category", fontsize=11, fontweight='bold', color=NAVY)
    ax1.legend(wedges, [c[:18] for c in cat_counts.index], loc="center left", bbox_to_anchor=(0.95, 0.5), fontsize=8, frameon=False)

    # Visual 1B: Inflow by Grievance Channel (Horizontal Bar)
    ax2 = fig.add_subplot(gs[1, 1:])
    top_channels = fact_df['channel'].value_counts().head(7)
    sns.barplot(x=top_channels.values, y=top_channels.index, palette="Blues_r", ax=ax2)
    ax2.set_title("Multi-Channel Grievance Inflow (Fragmentation Analysis)", fontsize=11, fontweight='bold', color=NAVY)
    ax2.set_xlabel("Complaint Volume", fontsize=9)
    for p in ax2.patches:
        ax2.annotate(f"{int(p.get_width()):,}", (p.get_width() + 500, p.get_y() + p.get_height()/2), va='center', fontsize=8)

    # Visual 1C: Monthly Inflow Trend & SLA Performance
    ax3 = fig.add_subplot(gs[2, :])
    monthly_trend = fact_df.groupby('created_month').agg(
        inflow=('complaint_id', 'count'),
        sla_comp=('within_sla', 'mean')
    ).reset_index()
    month_names = ['Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun', 'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec']
    monthly_trend['month_name'] = monthly_trend['created_month'].apply(lambda m: month_names[int(m)-1] if 1<=int(m)<=12 else str(m))
    
    ax3.plot(monthly_trend['month_name'], monthly_trend['inflow'], marker='o', linewidth=2.5, color=SKY, label="Complaint Inflow")
    ax3.set_ylabel("Monthly Inflow Volume", color=SKY, fontsize=9)
    ax3_twin = ax3.twinx()
    ax3_twin.plot(monthly_trend['month_name'], monthly_trend['sla_comp']*100, marker='s', linewidth=2, linestyle='--', color=EMERALD, label="SLA Compliance %")
    ax3_twin.set_ylabel("SLA Compliance %", color=EMERALD, fontsize=9)
    ax3_twin.set_ylim(50, 100)
    ax3.set_title("Monthly Grievance Inflow & SLA Compliance Trends", fontsize=11, fontweight='bold', color=NAVY)

    plt.savefig(os.path.join(VISUALS_DIR, "01_executive_kpi_overview.png"), dpi=250, bbox_inches='tight')
    plt.close()
    print("  Saved: 01_executive_kpi_overview.png")

    # ==========================================================================
    # VISUAL 2: WARD & GEOGRAPHIC PERFORMANCE
    # ==========================================================================
    print("[2/5] Generating Page 2: Ward & Geographic Analytics...")
    fig, axes = plt.subplots(1, 2, figsize=(16, 7), facecolor=LIGHT_BG)

    # 2A: Bottleneck Quadrant (Density vs Resolution Hours)
    zone_perf = fact_df.groupby('zone').agg(
        total=('complaint_id', 'count'),
        avg_hours=('resolution_time_hours', 'mean'),
        fake_rate=('is_fake_closure', 'mean')
    ).reset_index()
    
    sns.scatterplot(
        data=zone_perf, x='fake_rate', y='avg_hours', size='total', sizes=(200, 1500),
        hue='zone', palette='Set2', ax=axes[0], legend='brief'
    )
    axes[0].axhline(y=zone_perf['avg_hours'].mean(), color='gray', linestyle=':', label='City Avg Turnaround')
    axes[0].axvline(x=zone_perf['fake_rate'].mean(), color='red', linestyle=':', label='City Avg Fake Closure Rate')
    axes[0].set_title("Administrative Bottleneck Quadrant by Zone\\n(Turnaround vs Fake Closure Rate)", fontsize=11, fontweight='bold', color=NAVY)
    axes[0].set_xlabel("Fake Closure Rate", fontsize=9)
    axes[0].set_ylabel("Average Turnaround (Hours)", fontsize=9)
    axes[0].text(0.11, 48, "HIGH BOTTLENECK\\n(Slow + Suspicious)", color=CRIMSON, fontweight='bold', fontsize=8)

    # 2B: Zone SLA Compliance Breakdown (100% Stacked)
    zone_sla = fact_df.groupby(['zone', 'within_sla']).size().unstack(fill_value=0)
    zone_sla_pct = zone_sla.div(zone_sla.sum(axis=1), axis=0) * 100
    zone_sla_pct.plot(kind='barh', stacked=True, color=[CRIMSON, EMERALD], ax=axes[1])
    axes[1].set_title("Zone-wise Citizen Charter SLA Compliance (Within vs Breached)", fontsize=11, fontweight='bold', color=NAVY)
    axes[1].set_xlabel("Percentage of Complaints", fontsize=9)
    axes[1].legend(["SLA Breached (> Max)", "Within SLA Target"], loc="lower right", fontsize=8)

    plt.savefig(os.path.join(VISUALS_DIR, "02_ward_geographic_performance.png"), dpi=250, bbox_inches='tight')
    plt.close()
    print("  Saved: 02_ward_geographic_performance.png")

    # ==========================================================================
    # VISUAL 3: COMPLAINT LIFECYCLE FUNNEL
    # ==========================================================================
    print("[3/5] Generating Page 3: Complaint Lifecycle Funnel...")
    fig, axes = plt.subplots(1, 2, figsize=(16, 6.5), facecolor=LIGHT_BG)

    # 3A: Funnel Chart
    stages = funnel_df['lifecycle_stage'].tolist()[::-1]
    counts = funnel_df['complaints_count'].tolist()[::-1]
    convs = funnel_df['conversion_pct'].tolist()[::-1]
    
    bars = axes[0].barh(stages, counts, color=SKY, edgecolor=NAVY, height=0.55)
    axes[0].set_title("Complaint Lifecycle Redressal Funnel", fontsize=11, fontweight='bold', color=NAVY)
    axes[0].set_xlabel("Grievance Volume", fontsize=9)
    for i, (b, c, conv) in enumerate(zip(bars, counts, convs)):
        axes[0].text(b.get_width() * 0.5, b.get_y() + b.get_height()/2, f"{c:,} ({conv}%)",
                     ha='center', va='center', color='white', fontweight='bold', fontsize=9)

    # 3B: Turnaround Hours vs Citizen Charter SLA Target
    cat_sla_comp = fact_df.groupby('standard_category')['resolution_time_hours'].mean().reset_index()
    cat_sla_comp['target'] = cat_sla_comp['standard_category'].map(lambda c: dim_cat.set_index('category_name')['sla_target_hours'].get(c, 48.0))
    
    x_idx = np.arange(len(cat_sla_comp))
    width = 0.35
    axes[1].bar(x_idx - width/2, cat_sla_comp['resolution_time_hours'], width, label="Empirical Avg Hours", color=SKY)
    axes[1].bar(x_idx + width/2, cat_sla_comp['target'], width, label="Citizen Charter SLA Target", color=GOLD)
    axes[1].set_xticks(x_idx)
    axes[1].set_xticklabels([c[:12] for c in cat_sla_comp['standard_category']], rotation=45, ha='right', fontsize=8)
    axes[1].set_title("Empirical Turnaround vs. Citizen Charter SLA Target", fontsize=11, fontweight='bold', color=NAVY)
    axes[1].set_ylabel("Hours", fontsize=9)
    axes[1].legend(fontsize=8)

    plt.savefig(os.path.join(VISUALS_DIR, "03_complaint_lifecycle_funnel.png"), dpi=250, bbox_inches='tight')
    plt.close()
    print("  Saved: 03_complaint_lifecycle_funnel.png")

    # ==========================================================================
    # VISUAL 4: ACCOUNTABILITY & FAKE CLOSURE DETECTION
    # ==========================================================================
    print("[4/5] Generating Page 4: Accountability & Fake Closure Analytics...")
    fig, axes = plt.subplots(1, 2, figsize=(16, 6.5), facecolor=LIGHT_BG)

    # 4A: Zone Fake Closure Alert Ranking
    zone_fake = fact_df.groupby('zone')['is_fake_closure'].mean().sort_values(ascending=False) * 100
    bar_colors = [CRIMSON if v >= 10.0 else (AMBER if v >= 8.5 else EMERALD) for v in zone_fake.values]
    
    axes[0].barh(zone_fake.index[::-1], zone_fake.values[::-1], color=bar_colors[::-1], height=0.6)
    axes[0].axvline(x=10.0, color='red', linestyle='--', label='Critical Alert Threshold (10%)')
    axes[0].set_title("Zone-wise Fake Closure Anomaly Rate (Audit Priority)", fontsize=11, fontweight='bold', color=NAVY)
    axes[0].set_xlabel("Flagged Fake Closure Rate (%)", fontsize=9)
    axes[0].legend(loc='lower right', fontsize=8)
    for p in axes[0].patches:
        axes[0].annotate(f"{p.get_width():.1f}%", (p.get_width() + 0.2, p.get_y() + p.get_height()/2), va='center', fontsize=8)

    # 4B: Anomaly Category Distribution (Treemap / Pie)
    reasons = fact_df['anomaly_reason'].value_counts()
    reasons_non_normal = reasons[reasons.index != 'Normal']
    axes[1].pie(reasons_non_normal.values, labels=reasons_non_normal.index, autopct='%1.1f%%', startangle=140,
                colors=[CRIMSON, AMBER, GOLD, SLATE], wedgeprops=dict(edgecolor='white', width=0.6))
    axes[1].set_title("Breakdown of Flagged Municipal Anomaly Modes", fontsize=11, fontweight='bold', color=NAVY)

    plt.savefig(os.path.join(VISUALS_DIR, "04_accountability_fake_closure.png"), dpi=250, bbox_inches='tight')
    plt.close()
    print("  Saved: 04_accountability_fake_closure.png")

    # ==========================================================================
    # VISUAL 5: TEMPORAL INFLOW HEATMAP
    # ==========================================================================
    print("[5/5] Generating Page 5: Temporal Inflow Heatmap Matrix...")
    pivot_heat = heatmap_df.pivot(index='created_day_name', columns='created_hour', values='complaint_inflow_count')
    day_order = ['Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday', 'Sunday']
    pivot_heat = pivot_heat.reindex(day_order)

    plt.figure(figsize=(15, 6), facecolor=LIGHT_BG)
    sns.heatmap(pivot_heat, cmap="Blues", annot=False, cbar_kws={'label': 'Hourly Complaint Inflow Volume'})
    plt.title("Civic Complaint Inflow Heatmap Matrix (Day of Week vs. Hour of Day)", fontsize=12, fontweight='bold', color=NAVY)
    plt.xlabel("Hour of the Day (0:00 to 23:00)", fontsize=10)
    plt.ylabel("Day of the Week", fontsize=10)
    plt.savefig(os.path.join(VISUALS_DIR, "05_temporal_inflow_heatmap.png"), dpi=250, bbox_inches='tight')
    plt.close()
    print("  Saved: 05_temporal_inflow_heatmap.png")

    print("\n" + "=" * 80)
    print(f"ALL 5 POWER BI DASHBOARD VISUALS GENERATED IN: {VISUALS_DIR}")
    print("=" * 80)

if __name__ == "__main__":
    generate_all_visuals()
