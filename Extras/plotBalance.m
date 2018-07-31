data = csvread('testOutput.csv',2,1)

%data2 = csvread('M1graphMACD.csv');
x = length(data(:,2));
subplot(4,1,1);
plot(data(1:x, 2), data(1:x,9));
hold on;
grid
legend('Balance');

subplot(4,1,2);
scatter(data(1:x, 1), data(1:x,8));
legend('Profit');
%y = sum(data(1:x,7))
%y = y / length(data(1:x,8));
%plot(data(1:x, 1), y);
%P = polyfit(data(1:x, 1), data(1:x,8), 1);
 %yfit = P(1)*data(1:x, 1)+P(2);
 %   hold on;
  %  plot(data(1:x, 1),yfit,'r-.');

subplot(4,1,3);
%plot(data2(:,1), data2(:,4));
hold on;
%plot(data2(:,1), data2(:,5));
%plot(data2(:,1), zeros(1,x));
%plot(data(1:x, 1), b);
grid

%legend('MACD','Signq','ZERO');

subplot(4,1,4);
scatter(data(:, 1), data(:,3));
hold on
scatter(data(:, 1), data(:,4));
grid
legend('Ask Price', 'Bid Price');
