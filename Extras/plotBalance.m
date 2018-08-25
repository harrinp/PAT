clear; close all; clc;

data = csvread('testOutput.csv',2,1);

subplot(2,2,1);
plot(data(:,2), data(:,9));
hold on;
grid
legend('Balance');
title('Account Balance vs Time');
xlabel('Date (Sec from epoch)');
ylabel('Balance (Dollars)');


subplot(2,2,2);
scatter(data(:, 1), data(:,8));
legend('Profit');
title('Profit per Trade');

subplot(2,2,3);
scatter(data(:, 1), data(:,3), 'r', 's');
hold on
scatter(data(:, 1), data(:,4));
grid
legend('Ask Price', 'Bid Price');
title('Ask and Bid Prices vs Time');

subplot(2,2,4);
hist(data(:, 8), 50);
axis([-1200 1200]);
title('Trade Value Histogram');
