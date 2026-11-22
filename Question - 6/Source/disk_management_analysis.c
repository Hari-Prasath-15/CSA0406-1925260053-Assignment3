#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/*
 * Program: disk_management_analysis.c
 * Operating Systems - Assignment 3 - Question 6 (Disk Management)
 *
 * Performs theoretical capacity and reliability (data-loss probability)
 * analysis for a Hospital Records Server with 6 disks of 4 TB each.
 *
 * NOTE: This is a theoretical capacity and reliability analysis program.
 * It does NOT create or modify any real RAID array or disk partition.
 */

static double n_choose_k(int n, int k) {
    if (k < 0 || k > n) {
        return 0.0;
    }
    if (k == 0 || k == n) {
        return 1.0;
    }
    if (k > n - k) {
        k = n - k;
    }
    double res = 1.0;
    for (int i = 1; i <= k; i++) {
        res = res * (double)(n - k + i) / (double)i;
    }
    return res;
}

/* Probability of k or more independent disk failures out of n disks */
static double prob_at_least_k_failures(int n, int k, double p) {
    double prob_less_than_k = 0.0;
    for (int i = 0; i < k; i++) {
        double term = n_choose_k(n, i) * pow(p, i) * pow(1.0 - p, n - i);
        prob_less_than_k += term;
    }
    double loss = 1.0 - prob_less_than_k;
    return (loss < 0.0) ? 0.0 : loss;
}

static void write_raid_capacity_report(FILE *out, int num_disks, double disk_tb) {
    double raw_tb = (double)num_disks * disk_tb;
    double r0_tb  = (double)num_disks * disk_tb;
    double r1_nway_tb = disk_tb;                        /* Pure N-way mirror (1 copy usable) */
    double r1_pairs_tb = ((double)num_disks / 2.0) * disk_tb; /* 3 independent 2-disk mirrors */
    double r5_tb  = (double)(num_disks - 1) * disk_tb;
    double r10_tb = ((double)num_disks / 2.0) * disk_tb;
    double r6_tb  = (double)(num_disks - 2) * disk_tb;

    fprintf(out, "============================================================================\n");
    fprintf(out, " THEORETICAL RAID CAPACITY ANALYSIS (%d Disks x %.1f TB = %.1f TB Raw)\n",
            num_disks, disk_tb, raw_tb);
    fprintf(out, "============================================================================\n");
    fprintf(out, "Note: Theoretical calculation only (no physical RAID array created).\n\n");

    fprintf(out, "%-24s %-16s %-12s %-12s %-10s\n",
            "RAID Level", "Formula", "Usable (TB)", "Efficiency", "Fault Tol.");
    fprintf(out, "----------------------------------------------------------------------------\n");
    fprintf(out, "%-24s %-16s %-12.2f %6.2f%%     %-10s\n",
            "RAID 0 (Striping)", "N * C", r0_tb, (r0_tb / raw_tb) * 100.0, "0 disks");
    fprintf(out, "%-24s %-16s %-12.2f %6.2f%%     %-10s\n",
            "RAID 1 (6-Way Mirror)", "1 * C", r1_nway_tb, (r1_nway_tb / raw_tb) * 100.0, "5 disks");
    fprintf(out, "%-24s %-16s %-12.2f %6.2f%%     %-10s\n",
            "RAID 1 (3x 2-Disk Pairs)", "(N/2) * C", r1_pairs_tb, (r1_pairs_tb / raw_tb) * 100.0, "1/pair");
    fprintf(out, "%-24s %-16s %-12.2f %6.2f%%     %-10s\n",
            "RAID 5 (Single Parity)", "(N - 1) * C", r5_tb, (r5_tb / raw_tb) * 100.0, "1 disk");
    fprintf(out, "%-24s %-16s %-12.2f %6.2f%%     %-10s\n",
            "RAID 10 (Stripe+Mirror)", "(N/2) * C", r10_tb, (r10_tb / raw_tb) * 100.0, "1/pair");
    fprintf(out, "%-24s %-16s %-12.2f %6.2f%%     %-10s\n",
            "RAID 6 (Dual Parity)", "(N - 2) * C", r6_tb, (r6_tb / raw_tb) * 100.0, "2 disks");
    fprintf(out, "----------------------------------------------------------------------------\n");
    fprintf(out, "Assumptions:\n");
    fprintf(out, "  - All %d disks are identical %.1f TB drives (Raw Capacity = %.1f TB).\n",
            num_disks, disk_tb, raw_tb);
    fprintf(out, "  - RAID 10 is arranged as 3 mirrored pairs of 2 disks each, striped together.\n");
    fprintf(out, "  - Usable capacity shown is pre-filesystem-overhead decimal terabytes (TB).\n");
}

static void write_failure_probability_report(FILE *out, int num_disks, double p) {
    /* RAID 1 (standard 2-disk mirror): data loss if both disks fail */
    double p_loss_r1_pair = p * p;

    /* RAID 1 (6-way mirror): data loss only if all 6 disks fail */
    double p_loss_r1_6way = pow(p, num_disks);

    /* RAID 10 (3 mirrored pairs of 2 disks): fails if at least 1 pair loses both disks */
    int num_pairs = num_disks / 2;
    double p_pair_survives = 1.0 - (p * p);
    double p_loss_r10 = 1.0 - pow(p_pair_survives, num_pairs);

    /* RAID 5 (6 disks): fails if 2 or more disks fail */
    double p_loss_r5 = prob_at_least_k_failures(num_disks, 2, p);

    /* RAID 6 (6 disks): fails if 3 or more disks fail */
    double p_loss_r6 = prob_at_least_k_failures(num_disks, 3, p);

    fprintf(out, "============================================================================\n");
    fprintf(out, " THEORETICAL PROBABILITY OF DATA LOSS (Illustrative Model)\n");
    fprintf(out, "============================================================================\n");
    fprintf(out, "Stated Illustrative Assumption (NOT given in assignment PDF):\n");
    fprintf(out, "  - Annualized Failure Rate (AFR) per disk : p = %.4f (%.2f%% per year)\n",
            p, p * 100.0);
    fprintf(out, "  - Evaluation time period                 : 1 year\n");
    fprintf(out, "  - Statistical model                      : Binomial (independent disk failures)\n");
    fprintf(out, "  - Array size                             : N = %d disks\n\n", num_disks);

    fprintf(out, "%-28s %-26s %-14s %-12s\n",
            "Configuration", "Data-Loss Condition", "Probability", "Percentage");
    fprintf(out, "----------------------------------------------------------------------------\n");
    fprintf(out, "%-28s %-26s %-14.8f %.6f%%\n",
            "RAID 1 (2-Disk Mirror)", "Both disks fail (p^2)",
            p_loss_r1_pair, p_loss_r1_pair * 100.0);
    fprintf(out, "%-28s %-26s %-14.10f %.8f%%\n",
            "RAID 1 (6-Way Mirror)", "All 6 disks fail (p^6)",
            p_loss_r1_6way, p_loss_r1_6way * 100.0);
    fprintf(out, "%-28s %-26s %-14.8f %.6f%%\n",
            "RAID 10 (3 Mirrored Pairs)", "Any pair loses 2 disks",
            p_loss_r10, p_loss_r10 * 100.0);
    fprintf(out, "%-28s %-26s %-14.8f %.6f%%\n",
            "RAID 5 (6 Disks, 1 Parity)", ">= 2 disks fail",
            p_loss_r5, p_loss_r5 * 100.0);
    fprintf(out, "%-28s %-26s %-14.8f %.6f%%\n",
            "RAID 6 (6 Disks, 2 Parity)", ">= 3 disks fail",
            p_loss_r6, p_loss_r6 * 100.0);
    fprintf(out, "----------------------------------------------------------------------------\n");
    fprintf(out, "Key Comparisons (at p = %.2f%% per disk per year):\n", p * 100.0);
    fprintf(out, "  - 6-disk RAID 5 is %.2fx more likely to lose data than a 2-disk RAID 1.\n",
            p_loss_r5 / p_loss_r1_pair);
    fprintf(out, "  - 6-disk RAID 5 is %.2fx more likely to lose data than 6-disk RAID 10.\n",
            p_loss_r5 / p_loss_r10);
    fprintf(out, "  - 6-disk RAID 6 reduces data-loss risk by %.2fx compared to RAID 5.\n",
            p_loss_r5 / p_loss_r6);
    fprintf(out, "\nLimitation of Independence Assumption:\n");
    fprintf(out, "  Real-world disk failures are often correlated (same manufacturing batch,\n");
    fprintf(out, "  power surges, overheating, vibration, and heavy rebuild I/O stress).\n");
    fprintf(out, "  Therefore, actual failure probabilities can be higher than this model.\n");
}

int main(int argc, char *argv[]) {
    int num_disks = 6;
    double disk_tb = 4.0;
    double failure_rate = 0.02; /* Illustrative 2% annual disk failure rate */

    if (argc >= 4) {
        num_disks = atoi(argv[1]);
        disk_tb = atof(argv[2]);
        failure_rate = atof(argv[3]);
    }

    /* Input validation */
    if (num_disks < 4 || (num_disks % 2 != 0)) {
        fprintf(stderr, "Error: Number of disks must be an even integer >= 4 (for RAID 5, 6, 10).\n");
        return 1;
    }
    if (disk_tb <= 0.0) {
        fprintf(stderr, "Error: Disk capacity must be > 0 TB.\n");
        return 1;
    }
    if (failure_rate < 0.0 || failure_rate > 1.0) {
        fprintf(stderr, "Error: Disk failure rate p must be between 0.0 and 1.0.\n");
        return 1;
    }

    /* Print both reports to terminal */
    write_raid_capacity_report(stdout, num_disks, disk_tb);
    printf("\n");
    write_failure_probability_report(stdout, num_disks, failure_rate);

    /* Save results to results/ directory */
    FILE *fp_cap = fopen("results/raid_capacity.txt", "w");
    if (fp_cap != NULL) {
        write_raid_capacity_report(fp_cap, num_disks, disk_tb);
        fclose(fp_cap);
    } else {
        perror("Warning: Could not write results/raid_capacity.txt");
    }

    FILE *fp_prob = fopen("results/failure_probability.txt", "w");
    if (fp_prob != NULL) {
        write_failure_probability_report(fp_prob, num_disks, failure_rate);
        fclose(fp_prob);
    } else {
        perror("Warning: Could not write results/failure_probability.txt");
    }

    return 0;
}
